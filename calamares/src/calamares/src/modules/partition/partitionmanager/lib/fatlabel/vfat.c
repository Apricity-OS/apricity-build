/*  Copyright 1995 David C. Niemi
 *  Copyright 1996-2003,2005,2007-2009 Alain Knaff.
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
 *
 * vfat.c
 *
 * Miscellaneous VFAT-related functions
 */

#include "msdos.h"
#include "mtools.h"
#include "vfat.h"
#include "file.h"
#include "dirCache.h"
#include "file_name.h"
#include "match.h"
#include "directory.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static int unicode_read(struct unicode_char *in, wchar_t *out, int num)
{
	wchar_t *end_out = out + num;

	while(out < end_out)
	{
		*out = in->lchar | ((in->uchar) << 8);
		++out;
		++in;
	}

	return num;
}

static void clear_vfat(struct vfat_state *v)
{
	v->subentries = 0;
	v->status = 0;
	v->present = 0;
}

/* sum_shortname
 *
 * Calculate the checksum that results from the short name in *dir.
 *
 * The sum is formed by circularly right-shifting the previous sum
 * and adding in each character, from left to right, padding both
 * the name and extension to maximum length with spaces and skipping
 * the "." (hence always summing exactly 11 characters).
 *
 * This exact algorithm is required in order to remain compatible
 * with Microsoft Windows-95 and Microsoft Windows NT 3.5.
 * Thanks to Jeffrey Richter of Microsoft Systems Journal for
 * pointing me to the correct algorithm.
 *
 * David C. Niemi (niemi@tuxers.net) 95.01.19
 */
static unsigned char sum_shortname(const struct dos_name_t *dn)
{
	unsigned char sum = 0;
	const char *name = dn->base;
	const char *end = name + 11;

	for (sum = 0; name<end; ++name)
		sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *name;

	return sum;
}

/* check_vfat
 *
 * Inspect a directory and any associated VSEs.
 * Return 1 if the VSEs comprise a valid long file name,
 * 0 if not.
 */
static void check_vfat(struct vfat_state *v, struct directory *dir)
{
	struct dos_name_t dn;

	if (! v->subentries)
		return;

	memcpy(dn.base, (char *)dir->name, 8);
	memcpy(dn.ext, (char *)dir->ext, 3);

	if (v->sum != sum_shortname(&dn))
		return;

	if ((v->status & ((1 << v->subentries) - 1)) != (1 << v->subentries) - 1)
		return; /* missing entries */

	/* zero out byte following last entry, for good measure */
	v->name[VSE_NAMELEN * v->subentries] = 0;
	v->present = 1;
}


int clear_vses(struct Stream_t *Dir, int entrySlot, size_t last)
{
	struct dirCache_t *cache = allocDirCache(Dir, last);
	int error;

	if (!cache)
		return -1; // TODO: check return code

	struct direntry_t entry;
	entry.Dir = Dir;
	entry.entry = entrySlot;

	addFreeEntry(cache, entry.entry, last);

	for (; entry.entry < (signed int) last; ++entry.entry)
	{
		dir_read(&entry, &error);

		if(error)
		    return error;

		if(!entry.dir.name[0] || entry.dir.name[0] == DELMARK)
			break;

		entry.dir.name[0] = DELMARK;

		if (entry.dir.attr == 0xf)
			entry.dir.attr = '\0';

		low_level_dir_write(&entry);
	}

	return 0;
}

int write_vfat(struct Stream_t *Dir, struct dos_name_t *shortname, char *longname, int start, struct direntry_t *mainEntry)
{
	struct vfat_subentry *vse;
	int vse_id;
	int num_vses;
	wchar_t *c;
	struct direntry_t entry;
	struct dirCache_t *cache;
	wchar_t unixyName[13];
	struct doscp_t *cp = GET_DOSCONVERT(Dir);
	wchar_t wlongname[MAX_VNAMELEN+1];
	int wlen;

	if(longname)
	{
		entry.Dir = Dir;
		vse = (struct vfat_subentry *) &entry.dir;
		/* Fill in invariant part of vse */
		vse->attribute = 0x0f;
		vse->hash1 = vse->sector_l = vse->sector_u = 0;
		vse->sum = sum_shortname(shortname);

		wlen = native_to_wchar(longname, wlongname, MAX_VNAMELEN + 1, 0, 0);
		num_vses = (wlen + VSE_NAMELEN - 1) / VSE_NAMELEN;

		for (vse_id = num_vses; vse_id; --vse_id)
		{
			int end = 0;

			c = wlongname + (vse_id - 1) * VSE_NAMELEN;

			c += unicode_write(c, vse->text1, VSE1SIZE, &end);
			c += unicode_write(c, vse->text2, VSE2SIZE, &end);
			c += unicode_write(c, vse->text3, VSE3SIZE, &end);

			vse->id = (vse_id == num_vses) ? (vse_id | VSE_LAST) : vse_id;

			entry.entry = start + num_vses - vse_id;
			low_level_dir_write(&entry);
		}
	}
	else
	{
		num_vses = 0;
		wlongname[0]='\0';
	}

	cache = allocDirCache(Dir, start + num_vses + 1);

	if(!cache)
		return -1; // TODO: check return code

	unix_name(cp, shortname->base, shortname->ext, 0, unixyName);
	addUsedEntry(cache, start, start + num_vses + 1, wlongname, unixyName, &mainEntry->dir);
	low_level_dir_write(mainEntry);

	return start + num_vses;
}

int dir_write(struct direntry_t *entry)
{
	if (entry->entry == -3)
	{
		fprintf(stderr, "Attempt to write root directory pointer\n");
		return -2; // TODO: make caller check return code
	}

	struct dirCache_t *cache = allocDirCache(entry->Dir, entry->entry + 1);

	if (!cache)
	{
		fprintf(stderr, "Out of memory error in dir_write\n");
		return -1; // TODO: make caller check return code
	}

	struct dirCacheEntry_t *dce = cache->entries[entry->entry];

	if (dce)
	{
		if(entry->dir.name[0] == DELMARK)
			addFreeEntry(cache, dce->beginSlot, dce->endSlot);
		else
			dce->dir = entry->dir;
	}

	low_level_dir_write(entry);

	return 0;
}


/*
 * The following function translates a series of vfat_subentries into
 * data suitable for a dircache entry
 */
static  void parse_vses(struct direntry_t *entry, struct vfat_state *v)
{
	struct vfat_subentry *vse = (struct vfat_subentry *) &entry->dir;
	unsigned char id = vse->id & VSE_MASK;
	unsigned char last_flag = (vse->id & VSE_LAST);

	if (id > MAX_VFAT_SUBENTRIES)
	{
		fprintf(stderr, "parse_vses: invalid VSE ID %d at %d.\n", id, entry->entry);
		return;
	}

/* 950819: This code enforced finding the VSEs in order.  Well, Win95
 * likes to write them in *reverse* order for some bizarre reason!  So
 * we pretty much have to tolerate them coming in any possible order.
 * So skip this check, we'll do without it (What does this do, Alain?).
 *
 * 950820: Totally rearranged code to tolerate any order but to warn if
 * they are not in reverse order like Win95 uses.
 *
 * 950909: Tolerate any order. We recognize new chains by mismatching
 * checksums. In the event that the checksums match, new entries silently
 * overwrite old entries of the same id. This should accept all valid
 * entries, but may fail to reject invalid entries in some rare cases.
 */

	/* bad checksum, begin new chain */
	if(v->sum != vse->sum)
	{
		clear_vfat(v);
		v->sum = vse->sum;
	}

	v->status |= 1 << (id-1);

	if(last_flag)
		v->subentries = id;

	wchar_t* c = &(v->name[VSE_NAMELEN * (id-1)]);
	c += unicode_read(vse->text1, c, VSE1SIZE);
	c += unicode_read(vse->text2, c, VSE2SIZE);
	c += unicode_read(vse->text3, c, VSE3SIZE);

	if (last_flag)
		*c = '\0';	/* Null terminate long name */
}


static struct dirCacheEntry_t *vfat_lookup_loop_common(struct doscp_t *cp,
		struct direntry_t *direntry,
		struct dirCache_t *cache,
		int lookForFreeSpace,
		int *io_error)
{
	int initpos = direntry->entry + 1;
	struct vfat_state vfat;

	/* not yet cached */
	*io_error = 0;
	clear_vfat(&vfat);

	while(1)
	{
		++direntry->entry;

		int error;
		if(!dir_read(direntry, &error))
		{
			if(error)
			{
			    *io_error = error;
			    return NULL;
			}

			addFreeEntry(cache, initpos, direntry->entry);

			return addEndEntry(cache, direntry->entry);
		}

		if (direntry->dir.name[0] == '\0')
		{
				/* the end of the directory */
			if(lookForFreeSpace)
				continue;

			return addEndEntry(cache, direntry->entry);
		}

		if(direntry->dir.name[0] != DELMARK && direntry->dir.attr == 0x0f)
			parse_vses(direntry, &vfat);
		else
			/* the main entry */
			break;
	}

	/* If we get here, it's a short name FAT entry, maybe erased.
	 * thus we should make sure that the vfat structure will be
	 * cleared before the next loop run */

	/* deleted file */
	if (direntry->dir.name[0] == DELMARK)
		return addFreeEntry(cache, initpos, direntry->entry + 1);

	check_vfat(&vfat, &direntry->dir);

	if(!vfat.present)
		vfat.subentries = 0;

	/* mark space between last entry and this one as free */
	addFreeEntry(cache, initpos, direntry->entry - vfat.subentries);

	wchar_t newfile[13];
	if (direntry->dir.attr & 0x8)
	{
		/* Read entry as a label */
		wchar_t *ptr = newfile;
		ptr += dos_to_wchar(cp, direntry->dir.name, ptr, 8);
		ptr += dos_to_wchar(cp, direntry->dir.ext, ptr, 3);
		*ptr = '\0';
	}
	else
		unix_name(cp, direntry->dir.name, direntry->dir.ext, direntry->dir.Case, newfile);

	wchar_t* longname = vfat.present ? vfat.name : 0;

	return addUsedEntry(cache, direntry->entry - vfat.subentries, direntry->entry + 1, longname, newfile, &direntry->dir);
}

static struct dirCacheEntry_t *vfat_lookup_loop_for_read(struct doscp_t *cp, struct direntry_t *direntry, struct dirCache_t *cache, int *io_error)
{
	*io_error = 0;
	struct dirCacheEntry_t *dce = cache->entries[direntry->entry + 1];

	if(dce)
	{
		direntry->entry = dce->endSlot - 1;
		return dce;
	}
	else
		return vfat_lookup_loop_common(cp, direntry, cache, 0, io_error);
}


typedef enum result_t {
	RES_NOMATCH,
	RES_MATCH,
	RES_END,
	RES_ERROR
} result_t;


/*
 * 0 does not match
 * 1 matches
 * 2 end
 */
static result_t checkNameForMatch(struct direntry_t *direntry,
			struct dirCacheEntry_t *dce,
			const wchar_t *filename,
			int length,
			int flags)
{
	switch(dce->type)
	{
		case DCET_FREE:
			return RES_NOMATCH;

		case DCET_END:
			return RES_END;

		case DCET_USED:
			break;

		default:
			fprintf(stderr, "Unexpected entry type %d\n", dce->type);
			return RES_ERROR;
	}

	direntry->dir = dce->dir;

	/* make sure the entry is of an accepted type */
	if ((direntry->dir.attr & 0x8) && !(flags & ACCEPT_LABEL))
		return RES_NOMATCH;

	/*---------- multiple files ----------*/
	if (!((flags & MATCH_ANY) ||
			(dce->longName &&
			match(dce->longName, filename, direntry->name, 0, length)) ||
			match(dce->shortName, filename, direntry->name, 1, length)))
		return RES_NOMATCH;

	/* entry of non-requested type, has to come after name
	 * checking because of clash handling */
	if(IS_DIR(direntry) && !(flags & ACCEPT_DIR)) {
		if(!(flags & (ACCEPT_LABEL|MATCH_ANY|NO_MSG))) {
			char tmp[4*13+1];
			wchar_to_native(dce->shortName,tmp,13);
			fprintf(stderr, "Skipping \"%s\", is a directory\n",
				tmp);
		}
		return RES_NOMATCH;
	}

	if (!(direntry->dir.attr & (ATTR_LABEL | ATTR_DIR)) && !(flags & ACCEPT_PLAIN))
	{
		if (!(flags & (ACCEPT_LABEL|MATCH_ANY|NO_MSG)))
		{
			char tmp[4*13+1];
			wchar_to_native(dce->shortName,tmp,13);
			fprintf(stderr, "Skipping \"%s\", is not a directory\n", tmp);
		}

		return RES_NOMATCH;
	}

	return RES_MATCH;
}


/*
 * vfat_lookup looks for filenames in directory dir.
 * if a name if found, it is returned in outname
 * if applicable, the file is opened and its stream is returned in File
 */

int vfat_lookup(struct direntry_t *direntry, const char *filename, int length, int flags, char *shortname, char *longname)
{
	wchar_t wfilename[MAX_VNAMELEN+1];
	wchar_t *wfilenamep = wfilename;

	if(length == -1 && filename)
		length = strlen(filename);

	if(filename != NULL)
		length = native_to_wchar(filename, wfilename, MAX_VNAMELEN, filename+length, 0);
	else
	{
		wfilenamep = NULL;
		length = 0;
	}

	if (direntry->entry == -2)
		return -1;

	struct dirCache_t *cache = allocDirCache(direntry->Dir, direntry->entry+1);
	if(!cache)
	{
		fprintf(stderr, "Out of memory error in vfat_lookup [0]\n");
		return -2; // TODO: make caller check return code
	}

	struct dirCacheEntry_t *dce = NULL;
	result_t result;
	struct doscp_t *cp = GET_DOSCONVERT(direntry->Dir);
	do
	{
		int io_error;
		dce = vfat_lookup_loop_for_read(cp, direntry, cache, &io_error);

		if(!dce)
		{
			if (io_error)
				return -2;

			fprintf(stderr, "Out of memory error in vfat_lookup\n");

			return -2;
		}

		result = checkNameForMatch(direntry, dce, wfilename, length, flags);
	} while(result == RES_NOMATCH);

	if(result == RES_MATCH)
	{
		if(longname)
		{
			if(dce->longName)
				wchar_to_native(dce->longName, longname, MAX_VNAMELEN);
			else
				*longname ='\0';
		}

		if(shortname)
			wchar_to_native(dce->shortName, shortname, 12);

		direntry->beginSlot = dce->beginSlot;
		direntry->endSlot = dce->endSlot-1;
		return 0; /* file found */
	}

	direntry->entry = -2;
	return -1; /* no file found */
}

static struct dirCacheEntry_t *vfat_lookup_loop_for_insert(struct doscp_t *cp,
			struct direntry_t *direntry,
			int initpos,
			struct dirCache_t *cache)
{
	struct dirCacheEntry_t *dce = cache->entries[initpos];
	int io_error;

	if(dce && dce->type != DCET_END)
		return dce;

	direntry->entry = initpos - 1;

	dce = vfat_lookup_loop_common(cp, direntry, cache, 1, &io_error);

	if(!dce)
	{
		if (io_error)
			return NULL;

		fprintf(stderr, "Out of memory error in vfat_lookup_loop\n");

		return NULL;
	}

	return cache->entries[initpos];
}

static void accountFreeSlots(struct scan_state *ssp, struct dirCacheEntry_t *dce)
{
	if(ssp->got_slots)
		return;

	if(ssp->free_end != dce->beginSlot)
		ssp->free_start = dce->beginSlot;

	ssp->free_end = dce->endSlot;

	if(ssp->free_end - ssp->free_start >= ssp->size_needed)
	{
		ssp->got_slots = 1;
		ssp->slot = ssp->free_start + ssp->size_needed - 1;
	}
}

static void clear_scan(wchar_t *longname, int use_longname, struct scan_state *s)
{
	s->shortmatch = s->longmatch = s->slot = -1;
	s->free_end = s->got_slots = s->free_start = 0;

	if (use_longname)
		s->size_needed = 1 + (wcslen(longname) + VSE_NAMELEN - 1) / VSE_NAMELEN;
	else
		s->size_needed = 1;
}

/* lookup_for_insert replaces the old scandir function.  It directly
 * calls into vfat_lookup_loop, thus eliminating the overhead of the
 * normal vfat_lookup
 */
int lookupForInsert(struct Stream_t *Dir,
			struct direntry_t *direntry,
			struct dos_name_t *dosname,
			char *longname,
			struct scan_state *ssp,
			int ignore_entry,
			int source_entry,
			int pessimisticShortRename,
			int use_longname)
{
	wchar_t shortName[13];
	wchar_t wlongname[MAX_VNAMELEN+1];
	struct doscp_t *cp = GET_DOSCONVERT(Dir);

	native_to_wchar(longname, wlongname, MAX_VNAMELEN+1, 0, 0);
	clear_scan(wlongname, use_longname, ssp);

	int ignore_match = ignore_entry == -2;

	struct direntry_t entry;
	initializeDirentry(&entry, Dir);
	ssp->match_free = 0;

	/* hash bitmap of already encountered names.  Speeds up batch appends
	 * to huge directories, because in the best case, we only need to scan
	 * the new entries rather than the whole directory */
	struct dirCache_t *cache = allocDirCache(Dir, 1);
	if(!cache)
	{
		fprintf(stderr, "Out of memory error in lookupForInsert\n");
		return -1; // TODO: make caller check return code?
	}

	if(!ignore_match)
		unix_name(cp, dosname->base, dosname->ext, 0, shortName);

	 /* position _before_ the next answered entry */
	int pos = cache->nrHashed;

	if (source_entry >= 0 || (pos && isHashed(cache, wlongname)))
		pos = 0;
	else if(pos && !ignore_match && isHashed(cache, shortName))
	{
		if (pessimisticShortRename)
		{
			ssp->shortmatch = -2;
			return 1;
		}

		pos = 0;
	}
	else if(growDirCache(cache, pos) < 0)
	{
		fprintf(stderr, "Out of memory error in vfat_looup [0]\n");
		return -1; // TODO: make caller check return code?
	}

	struct dirCacheEntry_t *dce;
	do
	{
		dce = vfat_lookup_loop_for_insert(cp, &entry, pos, cache);

		switch(dce->type)
		{
			case DCET_FREE:
				accountFreeSlots(ssp, dce);
				break;

			case DCET_USED:
				if(!(dce->dir.attr & 0x8) && (signed int)dce->endSlot-1 == source_entry)
				   accountFreeSlots(ssp, dce);

				/* labels never match, neither does the
				 * ignored entry */
				if( (dce->dir.attr & 0x8) || ((signed int)dce->endSlot-1==ignore_entry))
					break;

				/* check long name */
				if((dce->longName && !wcscasecmp(dce->longName, wlongname)) || (dce->shortName && !wcscasecmp(dce->shortName, wlongname)))
				{
					ssp->longmatch = dce->endSlot - 1;
					/* long match is a reason for immediate stop */
					direntry->beginSlot = dce->beginSlot;
					direntry->endSlot = dce->endSlot-1;
					return 1;
				}

				/* Long name or not, always check for short name match */
				if (!ignore_match && !wcscasecmp(shortName, dce->shortName))
					ssp->shortmatch = dce->endSlot - 1;
				break;

			case DCET_END:
				break;
		}

		pos = dce->endSlot;

	} while(dce->type != DCET_END);

	if (ssp->shortmatch > -1)
		return 1;

	ssp->max_entry = dce->beginSlot;

	if (ssp->got_slots)
		return 6;	/* Success */

	/* Need more room.  Can we grow the directory? */
	if(!isRootDir(Dir))
		return 5;	/* OK, try to grow the directory */

	fprintf(stderr, "No directory slots\n");

	return -1;
}
