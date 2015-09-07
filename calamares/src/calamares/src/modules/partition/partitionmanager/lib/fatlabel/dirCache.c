/*  Copyright 1998,2001-2003,2007-2009 Alain Knaff.
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

#include "vfat.h"
#include "dirCache.h"
#include "file.h"
#include "mtools.h"

#include <wctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BITS_PER_INT (sizeof(unsigned int) * 8)

static unsigned int rol(unsigned int arg, int shift)
{
	arg &= 0xffffffff; /* for 64 bit machines */
	return (arg << shift) | (arg >> (32 - shift));
}

static int calcHash(wchar_t *name)
{
	unsigned long hash = 0;
	int i = 0;

	while(*name)
	{
		/* rotate it */
		hash = rol(hash,5); /* a shift of 5 makes sure we spread quickly
				     * over the whole width, moreover, 5 is
				     * prime with 32, which makes sure that
				     * successive letters cannot cover each
				     * other easily */
		wchar_t c = towupper(*name);

		hash ^=  (c * (c+2)) ^ (i * (i+2));
		hash &= 0xffffffff;

		i++;
		name++;
	}
	hash = hash * (hash + 2);

	/* the following two xors make sure all info is spread evenly over all
	 * bytes. Important if we only keep the low order bits later on */
	hash ^= (hash & 0xfff) << 12;
	hash ^= (hash & 0xff000) << 24;

	return hash;
}

static int addBit(unsigned int *bitmap, int hash, int checkOnly)
{
	int bit = 1 << (hash % BITS_PER_INT);
	int entry = (hash / BITS_PER_INT) % DC_BITMAP_SIZE;

	if(checkOnly)
		return bitmap[entry] & bit;

	bitmap[entry] |= bit;
	return 1;
}

static int _addHash(struct dirCache_t *cache, unsigned int hash, int checkOnly)
{
	return
		addBit(cache->bm0, hash, checkOnly) &&
		addBit(cache->bm1, rol(hash,12), checkOnly) &&
		addBit(cache->bm2, rol(hash,24), checkOnly);
}


static void addNameToHash(struct dirCache_t *cache, wchar_t *name)
{
	_addHash(cache, calcHash(name), 0);
}

static void hashDce(struct dirCache_t *cache, struct dirCacheEntry_t *dce)
{
	if(dce->beginSlot != cache->nrHashed)
		return;

	cache->nrHashed = dce->endSlot;

	if(dce->longName)
		addNameToHash(cache, dce->longName);

	addNameToHash(cache, dce->shortName);
}

int isHashed(struct dirCache_t *cache, wchar_t *name)
{
	return _addHash(cache, calcHash(name), 1);
}

int growDirCache(struct dirCache_t *cache, int slot)
{
	if(slot < 0)
	{
		fprintf(stderr, "Bad slot %d\n", slot);
		return -1; // TODO: check in caller
	}

	if( cache->nr_entries <= slot)
	{
		cache->entries = realloc(cache->entries, (slot+1) * 2 * sizeof(struct dirCacheEntry_t *));

		if(!cache->entries)
			return -1;

		for (int i = cache->nr_entries; i < (slot+1) * 2; i++)
			cache->entries[i] = 0;

		cache->nr_entries = (slot+1) * 2;
	}

	return 0;
}

struct dirCache_t *allocDirCache(struct Stream_t *Stream, int slot)
{
	if(slot < 0)
	{
		fprintf(stderr, "Bad slot %d\n", slot);
		return NULL;
	}

	struct dirCache_t **dcp = getDirCacheP(Stream);

	if(!*dcp)
	{
		*dcp = New(struct dirCache_t);

		if(!*dcp)
			return 0;

		(*dcp)->entries = NewArray((slot+1)*2+5, struct dirCacheEntry_t *);

		if(!(*dcp)->entries)
		{
			free(*dcp);
			return 0;
		}

		(*dcp)->nr_entries = (slot+1) * 2;
		memset( (*dcp)->bm0, 0, DC_BITMAP_SIZE);
		memset( (*dcp)->bm1, 0, DC_BITMAP_SIZE);
		memset( (*dcp)->bm2, 0, DC_BITMAP_SIZE);
		(*dcp)->nrHashed = 0;
	}
	else if(growDirCache(*dcp, slot) < 0)
		return 0;

	return *dcp;
}

static int freeDirCacheRange(struct dirCache_t *cache, unsigned int beginSlot, unsigned int endSlot)
{
	if(endSlot < beginSlot)
	{
		fprintf(stderr, "Bad slots %d %d in free range\n", beginSlot, endSlot);
		return -1; // TODO: check in caller
	}

	while(beginSlot < endSlot)
	{
		struct dirCacheEntry_t *entry = cache->entries[beginSlot];

		if(!entry)
		{
			beginSlot++;
			continue;
		}

		unsigned int clearEnd = entry->endSlot;

		if(clearEnd > endSlot)
			clearEnd = endSlot;

		unsigned int clearBegin = beginSlot;

		for(unsigned int i = clearBegin; i <clearEnd; i++)
			cache->entries[i] = 0;

		if(entry->endSlot == endSlot)
			entry->endSlot = beginSlot;
		else if(entry->beginSlot == beginSlot)
			entry->beginSlot = endSlot;
		else
		{
			fprintf(stderr, "Internal error, non contiguous de-allocation\n");
			fprintf(stderr, "%d %d\n", beginSlot, endSlot);
			fprintf(stderr, "%d %d\n", entry->beginSlot, entry->endSlot);
			return -1; // TODO: check in caller
		}

		if(entry->beginSlot == entry->endSlot)
		{
			if(entry->longName)
				free(entry->longName);

			if(entry->shortName)
				free(entry->shortName);

			free(entry);
		}

		beginSlot = clearEnd;
	}

	return 0;
}

static struct dirCacheEntry_t *allocDirCacheEntry(struct dirCache_t *cache, int beginSlot, int endSlot, dirCacheEntryType_t type)
{
	if(growDirCache(cache, endSlot) < 0)
		return 0;

	struct dirCacheEntry_t *entry = New(struct dirCacheEntry_t);

	if(!entry)
		return 0;

	entry->type = type;
	entry->longName = 0;
	entry->shortName = 0;
	entry->beginSlot = beginSlot;
	entry->endSlot = endSlot;

	freeDirCacheRange(cache, beginSlot, endSlot);

	for(int i = beginSlot; i<endSlot; i++)
		cache->entries[i] = entry;

	return entry;
}

struct dirCacheEntry_t *addUsedEntry(struct dirCache_t *cache, int beginSlot, int endSlot, wchar_t *longName, wchar_t *shortName, struct directory *dir)
{
	if(endSlot < beginSlot)
	{
		fprintf(stderr,"Bad slots %d %d in add used entry\n", beginSlot, endSlot);
		return NULL;
	}

	struct dirCacheEntry_t *entry = allocDirCacheEntry(cache, beginSlot, endSlot, DCET_USED);
	if(!entry)
		return 0;

	entry->beginSlot = beginSlot;
	entry->endSlot = endSlot;

	if(longName)
		entry->longName = wcsdup(longName);

	entry->shortName = wcsdup(shortName);
	entry->dir = *dir;

	hashDce(cache, entry);

	return entry;
}

static void mergeFreeSlots(struct dirCache_t *cache, int slot)
{
	if(slot == 0)
		return;

	struct dirCacheEntry_t *previous = cache->entries[slot-1];
	struct dirCacheEntry_t *next = cache->entries[slot];

	if(next && next->type == DCET_FREE && previous && previous->type == DCET_FREE)
	{
		for(unsigned int i = next->beginSlot; i < next->endSlot; i++)
			cache->entries[i] = previous;

		previous->endSlot = next->endSlot;
		free(next);
	}
}

struct dirCacheEntry_t *addFreeEntry(struct dirCache_t *cache, unsigned int beginSlot, unsigned int endSlot)
{
	if(beginSlot < cache->nrHashed)
		cache->nrHashed = beginSlot;

	if(endSlot < beginSlot)
	{
		fprintf(stderr, "Bad slots %d %d in add free entry\n", beginSlot, endSlot);
		return NULL;
	}

	if(endSlot == beginSlot)
		return 0;

	allocDirCacheEntry(cache, beginSlot, endSlot, DCET_FREE);
	mergeFreeSlots(cache, beginSlot);
	mergeFreeSlots(cache, endSlot);

	return cache->entries[beginSlot];
}

struct dirCacheEntry_t *addEndEntry(struct dirCache_t *cache, int pos)
{
	return allocDirCacheEntry(cache, pos, pos+1, DCET_END);
}

void freeDirCache(struct Stream_t *Stream)
{
	struct dirCache_t **dcp = getDirCacheP(Stream);
	struct dirCache_t *cache = *dcp;

	if(cache)
	{
		freeDirCacheRange(cache, 0, cache->nr_entries);
		free(cache);
		*dcp = 0;
	}
}
