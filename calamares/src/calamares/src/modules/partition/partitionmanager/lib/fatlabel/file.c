/*  Copyright 1996-1999,2001-2003,2007-2009 Alain Knaff.
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
#include "file.h"
#include "htable.h"
#include "dirCache.h"
#include "directory.h"
#include "init.h"
#include "fat.h"
#include "fs.h"
#include "llong.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

struct File_t
{
	struct Class_t *Class;
	int refs;
	struct Fs_t *Fs;	/* Filesystem that this fat file belongs to */
	struct Stream_t *Buffer;

	int (*map)(struct File_t *this, off_t where, size_t *len, int mode, off_t *res);
	size_t FileSize;

	size_t preallocatedSize;
	int preallocatedClusters;

	/* Absolute position of first cluster of file */
	unsigned int FirstAbsCluNr;

	/* Absolute position of previous cluster */
	unsigned int PreviousAbsCluNr;

	/* Relative position of previous cluster */
	unsigned int PreviousRelCluNr;
	struct direntry_t direntry;
	int hint;
	struct dirCache_t *dcp;

	unsigned int loopDetectRel;
	unsigned int loopDetectAbs;
};

static struct Class_t FileClass;
static struct hashtable *filehash;

static struct File_t *getUnbufferedFile(struct Stream_t *Stream)
{
	while(Stream->Class != &FileClass)
		Stream = Stream->Next;
	return (struct File_t *) Stream;
}

struct dirCache_t **getDirCacheP(struct Stream_t *Stream)
{
	return &getUnbufferedFile(Stream)->dcp;
}

struct direntry_t *getDirentry(struct Stream_t *Stream)
{
	return &getUnbufferedFile(Stream)->direntry;
}


static int recalcPreallocSize(struct File_t *This)
{
	struct Fs_t *Fs = This->Fs;
	int clus_size = Fs->cluster_size * Fs->sector_size;
	size_t currentClusters = (This->FileSize + clus_size - 1) / clus_size;
	size_t neededClusters = (This->preallocatedSize + clus_size - 1) / clus_size;
	int neededPrealloc = neededClusters - currentClusters;

	if (neededPrealloc < 0)
		neededPrealloc = 0;

	int r = fsPreallocateClusters(Fs, neededPrealloc - This->preallocatedClusters);
	if(r)
		return r;

	This->preallocatedClusters = neededPrealloc;

	return 0;
}

static int _loopDetect(unsigned int* oldrel, unsigned int rel, unsigned int* oldabs, unsigned int absol)
{
	if (*oldrel && rel > *oldrel && absol == *oldabs)
	{
		fprintf(stderr, "loop detected! oldrel=%d newrel=%d abs=%d\n", *oldrel, rel, absol);
		return -1;
	}

	if (rel >= 2 * *oldrel + 1)
	{
		*oldrel = rel;
		*oldabs = absol;
	}

	return 0;
}

static int loopDetect(struct File_t* This, unsigned int rel, unsigned int absol)
{
	return _loopDetect(&This->loopDetectRel, rel, &This->loopDetectAbs, absol);
}

static unsigned int _countBlocks(struct Fs_t *This, unsigned int block)
{
	unsigned int blocks = 0;
	unsigned int rel = 0;
	unsigned int oldabs = 0;
	unsigned int oldrel = 0;

	while (block <= This->last_fat && block != 1 && block)
	{
		blocks++;
		block = fatDecode(This, block);
		rel++;
		if (_loopDetect(&oldrel, rel, &oldabs, block) < 0)
			block = -1;
	}

	return blocks;
}

/* returns number of bytes in a directory.  Represents a file size, and
 * can hence be not bigger than 2^32
 */
static size_t countBytes(struct Stream_t *Dir, unsigned int block)
{
	struct Stream_t *Stream = GetFs(Dir);
	DeclareThis(struct Fs_t);

	return _countBlocks(This, block) * This->sector_size * This->cluster_size;
}

static int normal_map(struct File_t *This, off_t where, size_t *len, int mode, off_t *res)
{
	size_t end;
	int NrClu; /* number of clusters to read */

	struct Fs_t *Fs = This->Fs;

	*res = 0;
	int clus_size = Fs->cluster_size * Fs->sector_size;
	int offset = where % clus_size;

	if (mode == MT_READ)
		maximize(*len, This->FileSize - where);

	if (*len == 0)
		return 0;

	unsigned int NewCluNr;
	if (This->FirstAbsCluNr < 2)
	{
		if( mode == MT_READ || *len == 0)
		{
			*len = 0;
			return 0;
		}

		NewCluNr = get_next_free_cluster(This->Fs, 1);

		if (NewCluNr == 1 )
		{
			errno = ENOSPC;
			return -2;
		}

		hash_remove(filehash, (void *) This, This->hint);
		This->FirstAbsCluNr = NewCluNr;
		hash_add(filehash, (void *) This, &This->hint);
		fatAllocate(This->Fs, NewCluNr, Fs->end_fat);
	}

	unsigned int RelCluNr = where / clus_size;
	unsigned int CurCluNr;
	unsigned int AbsCluNr;

	if (RelCluNr >= This->PreviousRelCluNr)
	{
		CurCluNr = This->PreviousRelCluNr;
		AbsCluNr = This->PreviousAbsCluNr;
	}
	else
	{
		CurCluNr = 0;
		AbsCluNr = This->FirstAbsCluNr;
	}

	NrClu = (offset + *len - 1) / clus_size;
	while (CurCluNr <= RelCluNr + NrClu)
	{
		if (CurCluNr == RelCluNr)
		{
			/* we have reached the beginning of our zone. Save
			 * coordinates */
			This->PreviousRelCluNr = RelCluNr;
			This->PreviousAbsCluNr = AbsCluNr;
		}

		NewCluNr = fatDecode(This->Fs, AbsCluNr);

		if (NewCluNr == 1 || NewCluNr == 0)
		{
			fprintf(stderr,"Fat problem while decoding %d %x\n", AbsCluNr, NewCluNr);
			return -3; // TODO: check return code in caller
		}

		if (CurCluNr == RelCluNr + NrClu)
			break;

		if (NewCluNr > Fs->last_fat && mode == MT_WRITE)
		{
			/* if at end, and writing, extend it */
			NewCluNr = get_next_free_cluster(This->Fs, AbsCluNr);

			if (NewCluNr == 1) /* no more space */
			{
				errno = ENOSPC;
				return -2;
			}

			fatAppend(This->Fs, AbsCluNr, NewCluNr);
		}

		if (CurCluNr < RelCluNr && NewCluNr > Fs->last_fat)
		{
			*len = 0;
			return 0;
		}

		if (CurCluNr >= RelCluNr && NewCluNr != AbsCluNr + 1)
			break;

		CurCluNr++;
		AbsCluNr = NewCluNr;

		if(loopDetect(This, CurCluNr, AbsCluNr))
		{
			errno = EIO;
			return -2;
		}
	}

	maximize(*len, (1 + CurCluNr - RelCluNr) * clus_size - offset);

	end = where + *len;

	if((*len + offset) / clus_size + This->PreviousAbsCluNr-2 > Fs->num_clus)
	{
		fprintf(stderr, "cluster too big\n");
		return -3; // TODO: check return code in caller
	}

	*res = sectorsToBytes((struct Stream_t*)Fs, (This->PreviousAbsCluNr-2) * Fs->cluster_size + Fs->clus_start) + offset;
	return 1;
}

static int root_map(struct File_t *This, off_t where, size_t *len, int UNUSED(mode), off_t *res)
{
	struct Fs_t *Fs = This->Fs;

	if(Fs->dir_len * Fs->sector_size < (size_t) where)
	{
		*len = 0;
		errno = ENOSPC;
		return -2;
	}

	maximize(*len, Fs->dir_len * Fs->sector_size - where);

	if (*len == 0)
		return 0;

	*res = sectorsToBytes((struct Stream_t*)Fs, Fs->dir_start) + where;

	return 1;
}


static int read_file(struct Stream_t *Stream, char *buf, off_t iwhere, size_t len)
{
	DeclareThis(struct File_t);

	off_t pos;
	off_t where = truncBytes32(iwhere);

	struct Stream_t *Disk = This->Fs->Next;

	int err = This->map(This, where, &len, MT_READ, &pos);

	return (err <= 0) ? err : READS(Disk, buf, pos, len);
}

static int write_file(struct Stream_t *Stream, char *buf, off_t iwhere, size_t len)
{
	DeclareThis(struct File_t);

	off_t pos;

	struct Stream_t *Disk = This->Fs->Next;
	off_t where = truncBytes32(iwhere);

	const size_t requestedLen = len;

	int err = This->map(This, where, &len, MT_WRITE, &pos);

	if (err <= 0)
		return err;

	int ret = WRITES(Disk, buf, pos, len);

	if(ret > (signed int) requestedLen)
		ret = requestedLen;

	if (ret > 0 && where + ret > (off_t) This->FileSize)
		This->FileSize = where + ret;

	recalcPreallocSize(This);

	return ret;
}

static int free_file(struct Stream_t *Stream)
{
	DeclareThis(struct File_t);
	struct Fs_t *Fs = This->Fs;
	fsPreallocateClusters(Fs, -This->preallocatedClusters);
	free_stream(&This->direntry.Dir);
	freeDirCache(Stream);
	return hash_remove(filehash, (void *) Stream, This->hint);
}

static int flush_file(struct Stream_t *Stream)
{
	DeclareThis(struct File_t);
	struct direntry_t *entry = &This->direntry;

	if (isRootDir(Stream))
		return 0;

	if (This->FirstAbsCluNr != getStart(entry->Dir, &entry->dir))
	{
		set_word(entry->dir.start, This->FirstAbsCluNr & 0xffff);
		set_word(entry->dir.startHi, This->FirstAbsCluNr >> 16);
		dir_write(entry);
	}

	return 0;
}

static int pre_allocate_file(struct Stream_t *Stream, size_t isize)
{
	DeclareThis(struct File_t);

	size_t size = truncBytes32(isize);

	if (size > This->FileSize && size > This->preallocatedSize)
	{
		This->preallocatedSize = size;
		return recalcPreallocSize(This);
	}

	return 0;
}

static struct Class_t FileClass =
{
	read_file,
	write_file,
	flush_file, /* flush */
	free_file, /* free */
	0, /* get_geom */
	0, /*	get_file_data */
	pre_allocate_file,
	get_dosConvert_pass_through
};

static unsigned int getAbsCluNr(struct File_t *This)
{
	if(This->FirstAbsCluNr)
		return This->FirstAbsCluNr;
	if(isRootDir((struct Stream_t *) This))
		return 0;
	return 1;
}

static unsigned int func1(void *Stream)
{
	DeclareThis(struct File_t);
	return getAbsCluNr(This) ^ (long) This->Fs;
}

static unsigned int func2(void *Stream)
{
	DeclareThis(struct File_t);
	return getAbsCluNr(This);
}

static int comp(void *Stream, void *Stream2)
{
	DeclareThis(struct File_t);

	struct File_t *This2 = (struct File_t *) Stream2;

	return This->Fs != This2->Fs || getAbsCluNr(This) != getAbsCluNr(This2);
}

static void init_hash(void)
{
	static int is_initialised = 0;

	if(!is_initialised)
	{
		make_ht(func1, func2, comp, 20, &filehash);
		is_initialised = 1;
	}
}

static struct Stream_t *_internalFileOpen(struct Stream_t* Dir, unsigned int first, size_t size, struct direntry_t *entry)
{
	struct Stream_t *Stream = GetFs(Dir);
	DeclareThis(struct Fs_t);
	struct File_t Pattern;
	struct File_t *File;

	init_hash();
	This->refs++;

	if (first != 1)
	{
		/* we use the illegal cluster 1 to mark newly created files.
		 * do not manage those by hashtable */
		Pattern.Fs = This;
		Pattern.Class = &FileClass;
		Pattern.map = (first || (entry && !IS_DIR(entry))) ? normal_map : root_map;
		Pattern.FirstAbsCluNr = first;
		Pattern.loopDetectRel = 0;
		Pattern.loopDetectAbs = first;

		if (!hash_lookup(filehash, (T_HashTableEl) &Pattern, (T_HashTableEl **)&File, 0))
		{
			File->refs++;
			This->refs--;
			return (struct Stream_t*) File;
		}
	}

	File = New(struct File_t);
	if (!File)
		return NULL;

	File->dcp = 0;
	File->preallocatedClusters = 0;
	File->preallocatedSize = 0;

	/* memorize dir for date and attrib */
	File->direntry = *entry;
	if (entry->entry == -3)
		File->direntry.Dir = (struct Stream_t *) File; /* root directory */
	else
		copy_stream(File->direntry.Dir);

	File->Class = &FileClass;
	File->Fs = This;

	File->map = (first || (entry && !IS_DIR(entry))) ? normal_map : root_map;
	File->FirstAbsCluNr = (first == 1) ? 0 : first;
	File->loopDetectRel = 0;
	File->loopDetectAbs = 0;
	File->PreviousRelCluNr = 0xffff;
	File->FileSize = size;
	File->refs = 1;
	File->Buffer = 0;

	hash_add(filehash, (void *) File, &File->hint);

	return (struct Stream_t *) File;
}

struct Stream_t* OpenRoot(struct Stream_t* Dir)
{
	const unsigned int num = fat32RootCluster(Dir);

	struct direntry_t entry;
	memset(&entry, 0, sizeof(struct direntry_t));

	/* make the directory entry */
	entry.entry = -3;
	entry.name[0] = '\0';
	mk_entry_from_base("/", ATTR_DIR, num, 0, 0, &entry.dir);

	size_t size;
	if (num)
		size = countBytes(Dir, num);
	else
	{
		struct Fs_t* Fs = (struct Fs_t*) GetFs(Dir);
		size = Fs->dir_len * Fs->sector_size;
	}

	struct Stream_t* file = _internalFileOpen(Dir, num, size, &entry);
	bufferize(&file);

	return file;
}

int isRootDir(struct Stream_t *Stream)
{
	struct File_t *This = getUnbufferedFile(Stream);

	return This->map == root_map;
}
