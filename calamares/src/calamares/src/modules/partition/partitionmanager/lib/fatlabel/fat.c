/*  Copyright 1996-2006,2008,2009 Alain Knaff.
 *  Copyright 2010 Volker Lanz (vl@fidra.de)
 *  This file is part of mtools.
 *
 *  Mtools is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Mtools is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Mtools.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "msdos.h"
#include "stream.h"
#include "mtools.h"
#include "fs.h"
#include "file_name.h"
#include "init.h"
#include "force_io.h"
#include "llong.h"
#include "directory.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef long long fatBitMask;

struct FatMap_t {
	unsigned char *data;
	fatBitMask dirty;
	fatBitMask valid;
};

#define SECT_PER_ENTRY (sizeof(fatBitMask)*8)
#define ONE ((fatBitMask) 1)

static  int readSector(struct Fs_t *This, char *buf, unsigned int off, size_t size)
{
	return READS(This->Next, buf, sectorsToBytes((struct Stream_t *)This, off), size << This->sectorShift);
}

static  int forceReadSector(struct Fs_t *This, char *buf, unsigned int off, size_t size)
{
	return force_read(This->Next, buf, sectorsToBytes((struct Stream_t *)This, off), size << This->sectorShift);
}

static  int forceWriteSector(struct Fs_t *This, char *buf, unsigned int off,
				       size_t size)
{
	return force_write(This->Next, buf, sectorsToBytes((struct Stream_t*)This, off), size << This->sectorShift);
}

static struct FatMap_t *GetFatMap(struct Fs_t *Stream)
{
	Stream->fat_error = 0;

	int nr_entries = (Stream->fat_len + SECT_PER_ENTRY - 1) / SECT_PER_ENTRY;

	struct FatMap_t *map = NewArray(nr_entries, struct FatMap_t);

	if(!map)
		return NULL;

	int i;
	for(i=0; i< nr_entries; i++)
	{
		map[i].data = 0;
		map[i].valid = 0;
		map[i].dirty = 0;
	}

	return map;
}

static  int locate(struct Fs_t *Stream, size_t offset, int *slot, int *bit)
{
	if(offset >= Stream->fat_len)
		return -1;

	*slot = offset / SECT_PER_ENTRY;
	*bit = offset % SECT_PER_ENTRY;

	return 0;
}

static int fatReadSector(struct Fs_t *This, int sector, int slot, int bit, int dupe, fatBitMask bitmap)
{
	dupe = (dupe + This->primaryFat) % This->num_fat;
	int fat_start = This->fat_start + This->fat_len * dupe;

	int nr_sectors = (bitmap == 0) ? SECT_PER_ENTRY - bit % SECT_PER_ENTRY : 1;

	/* first, read as much as the buffer can give us */
	int ret = readSector(This, (char *)(This->FatMap[slot].data+(bit<<This->sectorShift)), fat_start+sector, nr_sectors);

	if(ret < 0)
		return 0;

	if((unsigned int) ret < This->sector_size)
	{
		/* if we got less than one sector's worth, insist to get at
		 * least one sector */
		ret = forceReadSector(This, (char*) (This->FatMap[slot].data + (bit << This->sectorShift)), fat_start+sector, 1);

		if(ret < (int) This->sector_size)
			return 0;

		return 1;
	}

	return ret >> This->sectorShift;
}

static int fatWriteSector(struct Fs_t *This, int sector, int slot, int bit, int dupe)
{
	dupe = (dupe + This->primaryFat) % This->num_fat;

	if (dupe && !This->writeAllFats)
		return This->sector_size;

	int fat_start = This->fat_start + This->fat_len * dupe;

	return forceWriteSector(This, (char*) (This->FatMap[slot].data + bit * This->sector_size), fat_start + sector, 1);
}

static unsigned char *loadSector(struct Fs_t *This, unsigned int sector, fatAccessMode_t mode, int recurs)
{
	int slot;
	int bit;

	if(locate(This,sector, &slot, &bit) < 0)
		return 0;

	if(!This->FatMap[slot].data)
	{
		/* allocate the storage space */
		This->FatMap[slot].data = malloc(This->sector_size * SECT_PER_ENTRY);

		if(!This->FatMap[slot].data)
			return 0;

		memset(This->FatMap[slot].data, 0xee, This->sector_size * SECT_PER_ENTRY);
	}

	if(! (This->FatMap[slot].valid & (ONE << bit)))
	{
		unsigned int i;
		int ret = -1;

		for(i=0; i< This->num_fat; i++)
		{
			/* read the sector */
			ret = fatReadSector(This, sector, slot, bit, i,
					    This->FatMap[slot].valid);

			if(ret == 0)
			{
				fprintf(stderr, "Error reading fat number %d\n", i);
				continue;
			}

			if(This->FatMap[slot].valid)
			    /* Set recurs if there have already been
			     * sectors loaded in this bitmap long
			     */
			    recurs = 1;

			break;
		}

		/* all copies bad.  Return error */
		if (ret == 0)
			return 0;

		for (i=0; (int) i < ret; i++)
			This->FatMap[slot].valid |= ONE << (bit + i);

		if(!recurs && ret == 1)
			/* do some prefetching, if we happened to only
			 * get one sector */
			loadSector(This, sector+1, mode, 1);
	}

	if(mode == FAT_ACCESS_WRITE)
	{
		This->FatMap[slot].dirty |= ONE << bit;
		This->fat_dirty = 1;
	}

	return This->FatMap[slot].data + (bit << This->sectorShift);
}


static unsigned char *getAddress(struct Fs_t *Stream, unsigned int num, fatAccessMode_t mode)
{
	int sector = num >> Stream->sectorShift;
	unsigned char *ret = 0;

	if(sector == Stream->lastFatSectorNr && Stream->lastFatAccessMode >= mode)
		ret = Stream->lastFatSectorData;

	if(!ret)
	{
		ret = loadSector(Stream, sector, mode, 0);

		if(!ret)
			return 0;

		Stream->lastFatSectorNr = sector;
		Stream->lastFatSectorData = ret;
		Stream->lastFatAccessMode = mode;
	}

	return ret + (num & Stream->sectorMask);
}


static int readByte(struct Fs_t *Stream, int start)
{
	unsigned char *address = getAddress(Stream, start, FAT_ACCESS_READ);
	return address ? *address : -1;
}


/*
 * Fat 12 encoding:
 *	|    byte n     |   byte n+1    |   byte n+2    |
 *	|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|
 *	| | | | | | | | | | | | | | | | | | | | | | | | |
 *	| n+0.0 | n+0.5 | n+1.0 | n+1.5 | n+2.0 | n+2.5 |
 *	    \_____  \____   \______/________/_____   /
 *	      ____\______\________/   _____/  ____\_/
 *	     /     \      \          /       /     \
 *	| n+1.5 | n+0.0 | n+0.5 | n+2.0 | n+2.5 | n+1.0 |
 *	|      FAT entry k      |    FAT entry k+1      |
 */

 /*
 * Get and decode a FAT (file allocation table) entry.  Returns the cluster
 * number on success or 1 on failure.
 */

static unsigned int fat12_decode(struct Fs_t *Stream, unsigned int num)
{
	unsigned int start = num * 3 / 2;
	int byte0 = readByte(Stream, start);
	int byte1 = readByte(Stream, start+1);

	if (num < 2 || byte0 < 0 || byte1 < 0 || num > Stream->num_clus+1)
	{
		fprintf(stderr,"[1] Bad address %d\n", num);
		return 1;
	}

	if (num & 1)
		return (byte1 << 4) | ((byte0 & 0xf0)>>4);
	else
		return ((byte1 & 0xf) << 8) | byte0;
}


/*
 * Puts a code into the FAT table.  Is the opposite of fat_decode().  No
 * sanity checking is done on the code.  Returns a 1 on error.
 */
static void fat12_encode(struct Fs_t *Stream, unsigned int num, unsigned int code)
{
	const int start = num * 3 / 2;
	unsigned char *address0 = getAddress(Stream, start, FAT_ACCESS_WRITE);
	unsigned char *address1 = getAddress(Stream, start+1, FAT_ACCESS_WRITE);

	if (num & 1)
	{
		/* (odd) not on byte boundary */
		*address0 = (*address0 & 0x0f) | ((code << 4) & 0xf0);
		*address1 = (code >> 4) & 0xff;
	}
	else
	{
		/* (even) on byte boundary */
		*address0 = code & 0xff;
		*address1 = (*address1 & 0xf0) | ((code >> 8) & 0x0f);
	}
}


/*
 * Fat 16 encoding:
 *	|    byte n     |   byte n+1    |
 *	|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|
 *	| | | | | | | | | | | | | | | | |
 *	|         FAT entry k           |
 */

static unsigned int fat16_decode(struct Fs_t *Stream, unsigned int num)
{
	unsigned char *address = getAddress(Stream, num << 1, FAT_ACCESS_READ);

	return address ? _WORD(address) : 1;
}

static void fat16_encode(struct Fs_t *Stream, unsigned int num, unsigned int code)
{
	unsigned char *address = getAddress(Stream, num << 1, FAT_ACCESS_WRITE);
	set_word(address, code);
}

static unsigned int fast_fat16_decode(struct Fs_t *Stream, unsigned int num)
{
	unsigned short *address = (unsigned short *) getAddress(Stream, num << 1, FAT_ACCESS_READ);
	return address ? *address : 1;
}

static void fast_fat16_encode(struct Fs_t *Stream, unsigned int num, unsigned int code)
{
	unsigned short *address =	(unsigned short *) getAddress(Stream, num << 1, FAT_ACCESS_WRITE);
	*address = code;
}

/*
 * Fat 32 encoding
 */
#define FAT32_HIGH 0xf0000000
#define FAT32_ADDR 0x0fffffff

static unsigned int fat32_decode(struct Fs_t *Stream, unsigned int num)
{
	unsigned char *address = getAddress(Stream, num << 2, FAT_ACCESS_READ);
	return address ? _DWORD(address) & FAT32_ADDR : 1;
}

static void fat32_encode(struct Fs_t *Stream, unsigned int num, unsigned int code)
{
	unsigned char *address = getAddress(Stream, num << 2, FAT_ACCESS_WRITE);
	set_dword(address,(code&FAT32_ADDR) | (_DWORD(address)&FAT32_HIGH));
}

static unsigned int fast_fat32_decode(struct Fs_t *Stream, unsigned int num)
{
	unsigned int *address = (unsigned int *) getAddress(Stream, num << 2, FAT_ACCESS_READ);
	return address ? *address & FAT32_ADDR : 1;
}

static void fast_fat32_encode(struct Fs_t *Stream, unsigned int num, unsigned int code)
{
	unsigned int *address = (unsigned int *) getAddress(Stream, num << 2, FAT_ACCESS_WRITE);
	*address = (*address & FAT32_HIGH) | (code & FAT32_ADDR);
}

/*
 * Write the FAT table to the disk.  Up to now the FAT manipulation has
 * been done in memory.  All errors are fatal.  (Might not be too smart
 * to wait till the end of the program to write the table.  Oh well...)
 */

int fat_write(struct Fs_t *This)
{
	unsigned int i;
	unsigned int j;
	unsigned int bit;
	unsigned int slot;
	int ret;
	int fat_start;

	if (!This->fat_dirty)
		return 0;

	unsigned int dups = This->num_fat;
	if (This->fat_error)
		dups = 1;

	for(i = 0; i < dups; i++)
	{
		j = 0;
		fat_start = This->fat_start + i*This->fat_len;

		for (slot = 0; j < This->fat_len; slot++)
		{
			if(!This->FatMap[slot].dirty)
			{
				j += SECT_PER_ENTRY;
				continue;
			}

			for(bit = 0; bit < SECT_PER_ENTRY && j < This->fat_len; bit++, j++)
			{
				if(!(This->FatMap[slot].dirty & (ONE << bit)))
					continue;

				ret = fatWriteSector(This,j,slot, bit, i);

				if (ret < (int) This->sector_size)
				{
					if (ret < 0)
					{
						perror("error in fat_write");
						return -1; // TODO: check in caller
					}

					fprintf(stderr, "end of file in fat_write\n");
					return -1; // TODO: check in caller
				}

				/* if last dupe, zero it out */
				if (i == dups - 1)
					This->FatMap[slot].dirty &= ~(ONE << bit);
			}
		}
	}

	/* write the info sector, if any */
	if(This->infoSectorLoc && This->infoSectorLoc != MAX32)
	{
		/* initialize info sector */
		struct InfoSector_t *infoSector = malloc(This->sector_size);

		set_dword(infoSector->signature1, INFOSECT_SIGNATURE1);

		memset(infoSector->filler1, 0, sizeof(infoSector->filler1));
		memset(infoSector->filler2, 0, sizeof(infoSector->filler2));

		set_dword(infoSector->signature2, INFOSECT_SIGNATURE2);
		set_dword(infoSector->pos, This->last);
		set_dword(infoSector->count, This->freeSpace);
		set_dword(infoSector->signature3, 0xaa55);

		if(forceWriteSector(This, (char *)infoSector, This->infoSectorLoc, 1) != (signed int) This->sector_size)
			fprintf(stderr,"Trouble writing the info sector\n");

		free(infoSector);
	}

	This->fat_dirty = 0;
	This->lastFatAccessMode = FAT_ACCESS_READ;

	return 0;
}

static void set_fat12(struct Fs_t *This)
{
	This->fat_bits = 12;
	This->end_fat = 0xfff;
	This->last_fat = 0xff6;
	This->fat_decode = fat12_decode;
	This->fat_encode = fat12_encode;
}

static char word_endian_test[] = { 0x34, 0x12 };

static void set_fat16(struct Fs_t *This)
{
	This->fat_bits = 16;
	This->end_fat = 0xffff;
	This->last_fat = 0xfff6;

	if(sizeof(unsigned short) == 2 && * (unsigned short *) word_endian_test == 0x1234) {
		This->fat_decode = fast_fat16_decode;
		This->fat_encode = fast_fat16_encode;
	}
	else
	{
		This->fat_decode = fat16_decode;
		This->fat_encode = fat16_encode;
	}
}

static char dword_endian_test[] = { 0x78, 0x56, 0x34, 0x12 };

static void set_fat32(struct Fs_t *This)
{
	This->fat_bits = 32;
	This->end_fat = 0xfffffff;
	This->last_fat = 0xffffff6;

	if(sizeof(unsigned int) == 4 && * (unsigned int *) dword_endian_test == 0x12345678)
	{
		This->fat_decode = fast_fat32_decode;
		This->fat_encode = fast_fat32_encode;
	}
	else
	{
		This->fat_decode = fat32_decode;
		This->fat_encode = fat32_encode;
	}
}

/*
 * Read the first sector of FAT table into memory.  Crude error detection on
 * wrong FAT encoding scheme.
 */
static int check_media_type(struct Fs_t *This, union bootsector UNUSED(*boot), unsigned int tot_sectors)
{
	This->num_clus = (tot_sectors - This->clus_start) / This->cluster_size;

	This->FatMap = GetFatMap(This);

	if (This->FatMap == NULL)
	{
		perror("alloc fat map");
		return -1;
	}

	unsigned char* address = getAddress(This, 0, FAT_ACCESS_READ);

	if(!address)
	{
		fprintf(stderr, "Could not read first FAT sector\n");
		return -1;
	}

	return 0;
}

static int fat_32_read(struct Fs_t *This, union bootsector *boot, unsigned int tot_sectors)
{
	int size;

	This->fat_len = DWORD(ext.fat32.bigFat);
	This->writeAllFats = !(boot->boot.ext.fat32.extFlags[0] & 0x80);
	This->primaryFat = boot->boot.ext.fat32.extFlags[0] & 0xf;
	This->rootCluster = DWORD(ext.fat32.rootCluster);
	This->clus_start = This->fat_start + This->num_fat * This->fat_len;

	/* read the info sector */
	size = This->sector_size;
	This->infoSectorLoc = WORD(ext.fat32.infoSector);

	if (This->sector_size >= 512 && This->infoSectorLoc && This->infoSectorLoc != MAX32)
	{
		struct InfoSector_t *infoSector = (struct InfoSector_t *) malloc(size);

		if (forceReadSector(This, (char *)infoSector, This->infoSectorLoc, 1) == (signed int) This->sector_size
				&& _DWORD(infoSector->signature1) == INFOSECT_SIGNATURE1
				&& _DWORD(infoSector->signature2) == INFOSECT_SIGNATURE2)
		{
			This->freeSpace = _DWORD(infoSector->count);
			This->last = _DWORD(infoSector->pos);
		}

		free(infoSector);
	}

	set_fat32(This);

	return check_media_type(This, boot, tot_sectors);
}

static int old_fat_read(struct Fs_t *This, union bootsector *boot, int UNUSED(config_fat_bits), size_t tot_sectors, int nodups)
{
	This->writeAllFats = 1;
	This->primaryFat = 0;
	This->dir_start = This->fat_start + This->num_fat * This->fat_len;
	This->clus_start = This->dir_start + This->dir_len;
	This->infoSectorLoc = MAX32;

	if(nodups)
		This->num_fat = 1;

	if(check_media_type(This,boot, tot_sectors))
		return -1;

	if(This->num_clus >= FAT12)
		set_fat16(This); /* third FAT byte must be 0xff */
	else
		set_fat12(This);

	return 0;
}

/*
 * Read the first sector of the  FAT table into memory and initialize
 * structures.
 */
int fat_read(struct Fs_t *This, union bootsector *boot, int fat_bits, size_t tot_sectors, int nodups)
{
	This->fat_error = 0;
	This->fat_dirty = 0;
	This->last = MAX32;
	This->freeSpace = MAX32;
	This->lastFatSectorNr = 0;
	This->lastFatSectorData = 0;

	return (This->fat_len) ? old_fat_read(This, boot, fat_bits, tot_sectors, nodups) : fat_32_read(This, boot, tot_sectors);
}


unsigned int fatDecode(struct Fs_t *This, unsigned int pos)
{
	unsigned int ret = This->fat_decode(This, pos);

	if(ret && (ret < 2 || ret > This->num_clus+1) && ret < This->last_fat)
	{
		fprintf(stderr, "Bad FAT entry %d at %d\n", ret, pos);
		This->fat_error++;
	}

	return ret;
}

/* append a new cluster */
void fatAppend(struct Fs_t *This, unsigned int pos, unsigned int newpos)
{
	This->fat_encode(This, pos, newpos);
	This->fat_encode(This, newpos, This->end_fat);

	if(This->freeSpace != MAX32)
		This->freeSpace--;
}

/* de-allocates the given cluster */
void fatDeallocate(struct Fs_t *This, unsigned int pos)
{
	This->fat_encode(This, pos, 0);
	if(This->freeSpace != MAX32)
		This->freeSpace++;
}

/* allocate a new cluster */
void fatAllocate(struct Fs_t *This, unsigned int pos, unsigned int value)
{
	This->fat_encode(This, pos, value);
	if(This->freeSpace != MAX32)
		This->freeSpace--;
}

unsigned int get_next_free_cluster(struct Fs_t *This, unsigned int last)
{
	unsigned int i;

	if (This->last != MAX32)
		last = This->last;

	if (last < 2 || last >= This->num_clus+1)
		last = 1;

	for (i = last + 1; i < This->num_clus + 2; i++)
	{
		unsigned int r = fatDecode(This, i);

		if(r == 1)
			goto exit_0;

		if (!r)
		{
			This->last = i;
			return i;
		}
	}

	for(i = 2; i < last + 1; i++)
	{
		unsigned int r = fatDecode(This, i);

		if(r == 1)
			goto exit_0;

		if (!r)
		{
			This->last = i;
			return i;
		}
	}


	fprintf(stderr,"No free cluster %d %d\n", This->preallocatedClusters, This->last);
	return 1;

 exit_0:
	fprintf(stderr, "FAT error\n");
	return 1;
}

int fat_error(struct Stream_t *Dir)
{
	struct Stream_t *Stream = GetFs(Dir);
	DeclareThis(struct Fs_t);

	if(This->fat_error)
		fprintf(stderr,"Fat error detected\n");

	return This->fat_error;
}

int fat32RootCluster(struct Stream_t *Dir)
{
	struct Stream_t *Stream = GetFs(Dir);
	DeclareThis(struct Fs_t);
	return This->fat_bits == 32 ? This->rootCluster : 0;
}

/*
 * Ensure that there is a minimum of total sectors free
 */
int getfreeMinClusters(struct Stream_t *Dir, size_t size)
{
	struct Stream_t *Stream = GetFs(Dir);

	DeclareThis(struct Fs_t);

	if(This->freeSpace != MAX32)
	{
		if (This->freeSpace >= size)
			return 1;
		else
		{
			fprintf(stderr, "Disk full\n");
			return 0;
		}
	}

	size_t total = 0;

	/* we start at the same place where we'll start later to actually
	 * allocate the sectors.  That way, the same sectors of the FAT, which
	 * are already loaded during getfreeMin will be able to be reused
	 * during get_next_free_cluster */
	unsigned int last = This->last;

	if (last < 2 || last >= This->num_clus + 2)
		last = 1;

	unsigned int i;
	for (i=last+1; i< This->num_clus+2; i++)
	{
		unsigned int r = fatDecode(This, i);

		if(r == 1)
			goto exit_0;

		if (!r)
			total++;

		if(total >= size)
			return 1;
	}

	for(i=2; i < last+1; i++)
	{
		unsigned int r = fatDecode(This, i);

		if(r == 1)
			goto exit_0;

		if (!r)
			total++;

		if(total >= size)
			return 1;
	}

	fprintf(stderr, "Disk full\n");
	return 0;

 exit_0:
	fprintf(stderr, "FAT error\n");
	return 0;
}

unsigned int getStart(struct Stream_t *Dir, struct directory *dir)
{
	struct Stream_t *Stream = GetFs(Dir);
	unsigned int first = START(dir);

	if(fat32RootCluster(Stream))
		first |= STARTHI(dir) << 16;

	return first;
}

int fs_free(struct Stream_t *Stream)
{
	DeclareThis(struct Fs_t);

	if (This->FatMap)
	{
		const int nr_entries = (This->fat_len + SECT_PER_ENTRY - 1) / SECT_PER_ENTRY;

		for (int i = 0; i < nr_entries; i++)
			if (This->FatMap[i].data)
				free(This->FatMap[i].data);

		free(This->FatMap);
	}

	if (This->cp)
		cp_close(This->cp);

	return 0;
}
