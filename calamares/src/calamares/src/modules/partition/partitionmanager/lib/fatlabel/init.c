/*  Copyright 1986-1992 Emmet P. Gray.
 *  Copyright 1996-2002,2006-2009 Alain Knaff.
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
#include "plain_io.h"
#include "buffer.h"
#include "file_name.h"
#include "fat.h"
#include "force_io.h"
#include "devices.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define BOOTSIZE 512

/*
 * Read the boot sector.  We glean the disk parameters from this sector.
 */
static int read_boot(struct Stream_t *Stream, union bootsector * boot, int size)
{
	/* read the first sector, or part of it */
	if(!size)
		size = BOOTSIZE;
	if(size > MAX_BOOT)
		size = MAX_BOOT;

	if (force_read(Stream, boot->characters, 0, size) != size)
		return -1;
	return 0;
}

static int fs_flush(struct Stream_t *Stream)
{
	DeclareThis(struct Fs_t);

	fat_write(This);
	return 0;
}

static struct doscp_t *get_dosConvert(struct Stream_t *Stream)
{
  DeclareThis(struct Fs_t);
  return This->cp;
}

static unsigned int log_2(int size)
{
	unsigned int i;

	for(i = 0; i < 24; i++)
		if (1 << i == size)
			return i;

	return 24;
}

static struct Class_t FsClass = {
	read_pass_through, /* read */
	write_pass_through, /* write */
	fs_flush,
	fs_free, /* free */
	0, /* set geometry */
	get_data_pass_through,
	0, /* pre allocate */
	get_dosConvert, /* dosconvert */
};

static int get_media_type(struct Stream_t *St, union bootsector *boot)
{
	int media;

	media = boot->boot.descr;
	if(media < 0xf0){
		char temp[512];
		/* old DOS disk. Media descriptor in the first FAT byte */
		/* old DOS disk always have 512-byte sectors */
		if (force_read(St,temp,(off_t) 512,512) == 512)
			media = (unsigned char) temp[0];
		else
			media = 0;
	} else
		media += 0x100;
	return media;
}

struct Stream_t *GetFs(struct Stream_t *Fs)
{
	while(Fs && Fs->Class != &FsClass)
		Fs = Fs->Next;
	return Fs;
}

static struct Stream_t *open_stream(const char* deviceName, int mode, struct device *out_dev, union bootsector *boot, int *media, size_t *maxSize)
{
	char errmsg[200];
	int r;

	snprintf(errmsg, 199, "Drive '%s:' not supported", deviceName);

	struct device dev;
	memset(&dev, 0, sizeof(dev));
	dev.name = deviceName;

	*out_dev = dev;

	struct Stream_t *Stream = SimpleFileOpen(out_dev, &dev, dev.name, mode, errmsg, 0, 1, maxSize);

	if (!Stream)
	{
		fprintf(stderr, "open_stream: opening file failed: %s.\n", errmsg);
	    return NULL;
	}

	/* read the boot sector */
	if ((r = read_boot(Stream, boot, out_dev->blocksize)) < 0)
	{
		snprintf(errmsg, 199, "init %s: could not read boot sector", deviceName);
		goto out;
	}

	if((*media = get_media_type(Stream, boot)) <= 0xf0 )
	{
		if (boot->boot.jump[2]=='L')
			snprintf(errmsg, 199, "diskette %s: is Linux LILO, not DOS", deviceName);
		else
			snprintf(errmsg, 199, "init %s: non DOS media", deviceName);
		goto out;
	}

	/* set new parameters, if needed */
	errno = 0;

	if (SET_GEOM(Stream, out_dev, &dev, *media, boot))
	{
		if (errno)
			snprintf(errmsg, 199, "Can't set disk parameters for %s: %s", deviceName, strerror(errno));
		else
			snprintf(errmsg, 199, "Can't set disk parameters for %s", deviceName);
		goto out;
	}

out:
	/* print error msg if needed */
	if (Stream == NULL)
	{
		free_stream(&Stream);
		fprintf(stderr, "%s\n", errmsg);
		return NULL;
	}

	return Stream;
}

struct Stream_t *fs_init(const char* deviceName, int mode)
{
	struct Fs_t *This = New(struct Fs_t);

	if (!This)
	{
		fprintf(stderr, "fs_init: Creating fs struct failed.\n");
		return NULL;
	}

	This->Direct = NULL;
	This->Next = NULL;
	This->refs = 1;
	This->Buffer = 0;
	This->Class = &FsClass;
	This->preallocatedClusters = 0;
	This->lastFatSectorNr = 0;
	This->lastFatAccessMode = 0;
	This->lastFatSectorData = 0;
	This->last = 0;

	struct device dev;
	union bootsector boot;
	int media = 0;
	size_t maxSize = 0;
	This->Direct = open_stream(deviceName, mode, &dev, &boot, &media, &maxSize);
	if(!This->Direct)
	{
		fprintf(stderr, "fs_init: opening stream failed.\n");
		return NULL;
	}

	This->sector_size = WORD_S(secsiz);

	if(This->sector_size > MAX_SECTOR)
	{
		fprintf(stderr,"init %s: sector size too big\n", deviceName);
		return NULL;
	}

	int i = log_2(This->sector_size);

	if(i == 24)
	{
		fprintf(stderr, "init %s: sector size (%d) not a small power of two\n", deviceName, This->sector_size);
		return NULL;
	}

	This->sectorShift = i;
	This->sectorMask = This->sector_size - 1;

	int cylinder_size = dev.heads * dev.sectors;
	This->serialized = 0;

	struct label_blk_t *labelBlock;
	/*
		* all numbers are in sectors, except num_clus
		* (which is in clusters)
		*/
	int nhs = 0;
	size_t tot_sectors = WORD_S(psect);
	if(!tot_sectors)
	{
		tot_sectors = DWORD_S(bigsect);
		nhs = DWORD_S(nhs);
	}
	else
		nhs = WORD_S(nhs);

	This->cluster_size = boot.boot.clsiz;
	This->fat_start = WORD_S(nrsvsect);
	This->fat_len = WORD_S(fatlen);
	This->dir_len = WORD_S(dirents) * MDIR_SIZE / This->sector_size;
	This->num_fat = boot.boot.nfat;

	if (This->fat_len)
		labelBlock = &boot.boot.ext.old.labelBlock;
	else
		labelBlock = &boot.boot.ext.fat32.labelBlock;

	if (labelBlock->dos4 == 0x29)
	{
		This->serialized = 1;
		This->serial_number = _DWORD(labelBlock->serial);
	}

	if (tot_sectors >= (maxSize >> This->sectorShift))
	{
		fprintf(stderr, "Big disks not supported on this architecture\n");
		return NULL; // TODO: make caller check return code
	}

	int disk_size = cylinder_size;

	if(disk_size > 256)
	{
		disk_size = dev.sectors;
		if(dev.sectors % 2)
			disk_size <<= 1;
	}

	if (disk_size % 2)
		disk_size *= 2;

	int blocksize = (!dev.blocksize || dev.blocksize < This->sector_size) ? This->sector_size : dev.blocksize;

	if (disk_size)
		This->Next = buf_init(This->Direct, 8 * disk_size * blocksize, disk_size * blocksize, This->sector_size);
	else
		This->Next = This->Direct;

	if (This->Next == NULL)
	{
		perror("init: allocate buffer");
		This->Next = This->Direct;
	}

	/* read the FAT sectors */
	if(fat_read(This, &boot, dev.fat_bits, tot_sectors, dev.use_2m & 0x7f))
	{
		fprintf(stderr, "fs_init: Reading FAT failed.\n");
		This->num_fat = 1;
		free_stream(&This->Next);
		free(This->Next);
		return NULL;
	}

	/* Set the codepage */
	This->cp = cp_open(dev.codepage);
	if(This->cp == NULL)
	{
		fprintf(stderr, "fs_init: Setting code page failed.\n");
		fs_free((struct Stream_t *)This);
		free_stream(&This->Next);
		free(This->Next);
		return NULL;
	}

	return (struct Stream_t*) This;
}

int fs_close(struct Stream_t* Stream)
{
	DeclareThis(struct Fs_t);

	int rval = SimpleFileClose(This->Direct);

	if (This->Next != This->Direct)
	{
		free_stream(&This->Next);
		free(This->Next);
	}

	fs_free((struct Stream_t *)This);
	free(This);

	return rval;
}

int fsPreallocateClusters(struct Fs_t *Fs, long size)
{
	if(size > 0 && getfreeMinClusters((struct Stream_t *)Fs, size) != 1)
		return -1;

	Fs->preallocatedClusters += size;
	return 0;
}
