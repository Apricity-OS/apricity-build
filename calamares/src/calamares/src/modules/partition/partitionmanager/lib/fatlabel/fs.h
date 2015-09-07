/*  Copyright 1996,1997,2001,2002,2007,2009 Alain Knaff.
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

#ifndef MTOOLS_FS_H
#define MTOOLS_FS_H

struct Stream_t;
struct Class_t;
struct doscp_t;

struct FsPublic_t
{
	struct Class_t *Class;
	int refs;
	struct Stream_t *Next;
	struct Stream_t *Buffer;

	int serialized;
	unsigned long serial_number;
	int cluster_size;
	unsigned int sector_size;
};

typedef enum fatAccessMode_t
{
	FAT_ACCESS_READ,
	FAT_ACCESS_WRITE
} fatAccessMode_t;

struct Fs_t
{
	struct Class_t *Class;
	int refs;
	struct Stream_t *Next;
	struct Stream_t *Buffer;

	int serialized;
	unsigned long serial_number;
	unsigned int cluster_size;
	unsigned int sector_size;
	int fat_error;

	unsigned int (*fat_decode)(struct Fs_t *This, unsigned int num);
	void (*fat_encode)(struct Fs_t *This, unsigned int num, unsigned int code);

	struct Stream_t *Direct;
	int fat_dirty;
	unsigned int fat_start;
	unsigned int fat_len;

	unsigned int num_fat;
	unsigned int end_fat;
	unsigned int last_fat;
	int fat_bits; /* must be signed, because we use negative values for special purposes */
	struct FatMap_t *FatMap;

	unsigned int dir_start;
	unsigned int dir_len;
	unsigned int clus_start;

	unsigned int num_clus;

	/* fat 32 */
	unsigned int primaryFat;
	unsigned int writeAllFats;
	unsigned int rootCluster;
	unsigned int infoSectorLoc;
	unsigned int last; /* last sector allocated, or MAX32 if unknown */
	unsigned int freeSpace; /* free space, or MAX32 if unknown */
	int preallocatedClusters;

	int lastFatSectorNr;
	unsigned char *lastFatSectorData;
	fatAccessMode_t lastFatAccessMode;
	int sectorMask;
	int sectorShift;

	struct doscp_t *cp;
};

int fs_free(struct Stream_t *Stream);
unsigned int get_next_free_cluster(struct Fs_t *Fs, unsigned int last);
unsigned int fatDecode(struct Fs_t *This, unsigned int pos);
void fatAppend(struct Fs_t *This, unsigned int pos, unsigned int newpos);
void fatDeallocate(struct Fs_t *This, unsigned int pos);
void fatAllocate(struct Fs_t *This, unsigned int pos, unsigned int value);
int fat_read(struct Fs_t *This, union bootsector *boot, int fat_bits, size_t tot_sectors, int nodups);
int fat_write(struct Fs_t *This);
struct Stream_t* fs_init(const char* deviceName, int mode);
int fat_error(struct Stream_t *Dir);
int fat32RootCluster(struct Stream_t *Dir);

#endif
