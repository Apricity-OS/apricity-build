/*  Copyright 1997,1999,2001-2003,2008,2009 Alain Knaff.
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

#ifndef MTOOLS_DIRCACHE_H
#define MTOOLS_DIRCACHE_H

#include "directory.h"

#include <wchar.h>


typedef enum {
	DCET_FREE,
	DCET_USED,
	DCET_END
} dirCacheEntryType_t;

#define DC_BITMAP_SIZE 128

struct dirCacheEntry_t {
	dirCacheEntryType_t type;
	unsigned int beginSlot;
	unsigned int endSlot;
	wchar_t *shortName;
	wchar_t *longName;
	struct directory dir;
};

struct dirCache_t {
	struct dirCacheEntry_t **entries;
	int nr_entries;
	unsigned int nrHashed;
	unsigned int bm0[DC_BITMAP_SIZE];
	unsigned int bm1[DC_BITMAP_SIZE];
	unsigned int bm2[DC_BITMAP_SIZE];
};

int isHashed(struct dirCache_t *cache, wchar_t *name);
int growDirCache(struct dirCache_t *cache, int slot);
struct dirCache_t *allocDirCache(struct Stream_t *Stream, int slot);
struct dirCacheEntry_t *addUsedEntry(struct dirCache_t *Stream, int begin, int end, wchar_t *longName, wchar_t *shortName, struct directory *dir);
void freeDirCache(struct Stream_t *Stream);
struct dirCacheEntry_t *addFreeEntry(struct dirCache_t *Stream, unsigned int begin, unsigned int end);
struct dirCacheEntry_t *addEndEntry(struct dirCache_t *Stream, int pos);
#endif
