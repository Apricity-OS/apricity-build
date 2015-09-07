/*  Copyright 1995 David C. Niemi
 *  Copyright 1996-2002,2008,2009 Alain Knaff.
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

#include "directory.h"

#include "msdos.h"
#include "stream.h"
#include "mtools.h"
#include "file.h"
#include "fs.h"
#include "file_name.h"
#include "fat.h"
#include "init.h"
#include "force_io.h"
#include "buffer.h"
#include "nameclash.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static const char* short_illegals = ";+=[]',\"*\\<>/?:|";
static const char* long_illegals = "\"*\\<>/?:|\005";


/*
 * Read a directory entry into caller supplied buffer
 */
struct directory *dir_read(struct direntry_t *entry, int *error)
{
	int n;
	*error = 0;
	if((n=force_read(entry->Dir, (char *) (&entry->dir),
			 (off_t) entry->entry * MDIR_SIZE,
			 MDIR_SIZE)) != MDIR_SIZE) {
		if (n < 0) {
			*error = -1;
		}
		return NULL;
	}
	return &entry->dir;
}

/*
 * Make a subdirectory grow in length.  Only subdirectories (not root)
 * may grow.  Returns a 0 on success, 1 on failure (disk full), or -1
 * on error.
 */

int dir_grow(struct Stream_t *Dir, int size)
{
	struct Stream_t *Stream = GetFs(Dir);
	DeclareThis(struct FsPublic_t);
	int ret;
	int buflen;
	char *buffer;

	if (!getfreeMinClusters(Dir, 1))
		return -1;

	buflen = This->cluster_size * This->sector_size;

	if(! (buffer=malloc(buflen)) ){
		perror("dir_grow: malloc");
		return -1;
	}

	memset((char *) buffer, '\0', buflen);
	ret = force_write(Dir, buffer, (off_t) size * MDIR_SIZE, buflen);
	free(buffer);
	if(ret < buflen)
		return -1;
	return 0;
}


void low_level_dir_write(struct direntry_t *entry)
{
	force_write(entry->Dir,
		    (char *) (&entry->dir),
		    (off_t) entry->entry * MDIR_SIZE, MDIR_SIZE);
}


/*
 * Make a directory entry.  Builds a directory entry based on the
 * name, attribute, starting cluster number, and size.  Returns a pointer
 * to a static directory structure.
 */

struct directory *mk_entry(const struct dos_name_t *dn, char attr,
			   unsigned int fat, size_t size, time_t date,
			   struct directory *ndir)
{
	struct tm *now;
	time_t date2 = date;
	unsigned char hour, min_hi, min_low, sec;
	unsigned char year, month_hi, month_low, day;

	now = localtime(&date2);
	dosnameToDirentry(dn, ndir);
	ndir->attr = attr;
	ndir->ctime_ms = 0;
	hour = now->tm_hour << 3;
	min_hi = now->tm_min >> 3;
	min_low = now->tm_min << 5;
	sec = now->tm_sec / 2;
	ndir->ctime[1] = ndir->time[1] = hour + min_hi;
	ndir->ctime[0] = ndir->time[0] = min_low + sec;
	year = (now->tm_year - 80) << 1;
	month_hi = (now->tm_mon + 1) >> 3;
	month_low = (now->tm_mon + 1) << 5;
	day = now->tm_mday;
	ndir -> adate[1] = ndir->cdate[1] = ndir->date[1] = year + month_hi;
	ndir -> adate[0] = ndir->cdate[0] = ndir->date[0] = month_low + day;

	set_word(ndir->start, fat & 0xffff);
	set_word(ndir->startHi, fat >> 16);
	set_dword(ndir->size, size);
	return ndir;
}

/*
 * Make a directory entry from base name. This is supposed to be used
 * from places such as mmd for making special entries (".", "..", "/", ...)
 * Thus it doesn't bother with character set conversions
 */
struct directory *mk_entry_from_base(const char *base, char attr,
				     unsigned int fat, size_t size, time_t date,
				     struct directory *ndir)
{
	struct dos_name_t dn;
	strncpy(dn.base, base, 8);
	strncpy(dn.ext, "   ", 3);
	return mk_entry(&dn, attr, fat, size, date, ndir);
}

void bufferize(struct Stream_t **Dir)
{
	struct Stream_t *BDir;

	if(!*Dir)
		return;

	BDir = buf_init(*Dir, 64*16384, 512, MDIR_SIZE);

	if(!BDir)
	{
		free_stream(Dir);
		*Dir = NULL;
	}
	else
		*Dir = BDir;
}

void initializeDirentry(struct direntry_t *entry, struct Stream_t *Dir)
{
	entry->entry = -1;
	entry->Dir = Dir;
	entry->beginSlot = 0;
	entry->endSlot = 0;
}

int isNotFound(struct direntry_t *entry)
{
	return entry->entry == -2;
}

static int isSpecial(const char *name)
{
	return name && (name[0] == '\0' || !strcmp(name, ".") || !strcmp(name, "..")) ? 1 : 0;
}

static int convert_to_shortname(struct doscp_t *cp, struct ClashHandling_t *ch, const char *un, struct dos_name_t *dn)
{
	int mangled;

	/* Then do conversion to dn */
	ch->name_converter(cp, un, &mangled, dn);
	dn->sentinel = '\0';

	return mangled;
}

static int contains_illegals(const char *string, const char *illegals, int len)
{
	for (; *string && len--; string++)
		if ((*string < ' ' && *string != '\005' && !(*string & 0x80)) || strchr(illegals, *string))
			return 1;

	return 0;
}

static int is_reserved(char *ans, int islong)
{
	unsigned int i;
	static const char *dev3[] = {"CON", "AUX", "PRN", "NUL", "   "};
	static const char *dev4[] = {"COM", "LPT" };

	for (i = 0; i < sizeof(dev3) / sizeof(*dev3); i++)
		if (!strncasecmp(ans, dev3[i], 3) && ((islong && !ans[3]) || (!islong && !strncmp(ans + 3, "     ", 5))))
			return 1;

	for (i = 0; i < sizeof(dev4)/sizeof(*dev4); i++)
		if (!strncasecmp(ans, dev4[i], 3) && (ans[3] >= '1' && ans[3] <= '4') && ((islong && !ans[4])
				|| (!islong && !strncmp(ans + 4, "    ", 4))))
			return 1;

	return 0;
}

static clash_action get_slots(struct Stream_t *Dir,
					 struct dos_name_t *dosname,
					 char *longname,
					 struct scan_state *ssp,
					 struct ClashHandling_t *ch)
{
	struct direntry_t entry;
	int pessimisticShortRename = 0;

	entry.Dir = Dir;

	if ((is_reserved(longname, 1)) || longname[strspn(longname, ". ")] == '\0'
		|| contains_illegals(longname, long_illegals, 1024)
		|| is_reserved(dosname->base, 0)
		|| contains_illegals(dosname->base, short_illegals, 11))
	{
		return NAMEMATCH_ERROR;
	}
	else
	{
		switch (lookupForInsert(Dir,
					&entry,
					dosname, longname, ssp,
					ch->ignore_entry,
					ch->source_entry,
					pessimisticShortRename &&
					ch->use_longname,
					ch->use_longname)) {
			case -1:
				return NAMEMATCH_ERROR;

			case 0: /* Single-file error error or skip request */
				return NAMEMATCH_SKIP;

			case 5: /* Grew directory, try again */
				return NAMEMATCH_GREW;

			case 6:
				return NAMEMATCH_SUCCESS; /* Success */
		}
	}

	return NAMEMATCH_ERROR;
}

static int write_slots(struct Stream_t *Dir, struct dos_name_t *dosname, char *longname, struct scan_state *ssp, write_data_callback *cb, int Case)
{
	/* write the file */
	if (fat_error(Dir))
		return -1;

	struct direntry_t entry;

	entry.Dir = Dir;
	entry.entry = ssp->slot;

	native_to_wchar(longname, entry.name, MAX_VNAMELEN, 0, 0);

	entry.name[MAX_VNAMELEN] = '\0';
	entry.dir.Case = Case & (EXTCASE | BASECASE);

	if (cb(dosname, &entry) < 0)
		return -2;

	if ((ssp->size_needed > 1) && (ssp->free_end - ssp->free_start >= ssp->size_needed))
		ssp->slot = write_vfat(Dir, dosname, longname, ssp->free_start, &entry);
	else
	{
		ssp->size_needed = 1;
		write_vfat(Dir, dosname, 0, ssp->free_start, &entry);
	}

	return 0;	/* Successfully wrote the file */
}

int mwrite_one(struct Stream_t *Dir, const char *argname, write_data_callback *cb, struct ClashHandling_t *ch)
{
	if (argname == NULL)
		return -1;

	if (isSpecial(argname))
	{
		fprintf(stderr, "Cannot create entry named . or ..\n");
		return -1;
	}

	/* Copy original argument dstname to working value longname */
	char longname[VBUFSIZE];
	strncpy(longname, argname, VBUFSIZE - 1);

	struct doscp_t *cp = GET_DOSCONVERT(Dir);
	struct dos_name_t dosname;
	ch->use_longname = convert_to_shortname(cp, ch, longname, &dosname);

	ch->action[0] = ch->namematch_default[0];
	ch->action[1] = ch->namematch_default[1];

	int expanded = 0;
	clash_action ret;

	for (;;)
	{
		struct scan_state scan;

		switch((ret = get_slots(Dir, &dosname, longname, &scan, ch)))
		{
			case NAMEMATCH_ERROR:
			case NAMEMATCH_SKIP:
				return -1;

			case NAMEMATCH_GREW:
				/* No collision, and not enough slots. Try to grow the directory */
				if (expanded)
				{
					/* Already tried this once, no good */
					fprintf(stderr, "No directory slots\n");
					return -1;
				}
				expanded = 1;

				if (dir_grow(Dir, scan.max_entry))
					return -1;

				continue;

			case NAMEMATCH_SUCCESS:
				return write_slots(Dir, &dosname, longname, &scan, cb, ch->use_longname);

			default:
				fprintf(stderr, "Internal error: clash_action=%d\n", ret);
				return -1;
		}
	}

	return ret;
}

void init_clash_handling(struct ClashHandling_t *ch)
{
	ch->ignore_entry = -1;
	ch->source_entry = -2;
	ch->nowarn = 0;	/*Don't ask, just do default action if name collision */
	ch->namematch_default[0] = NAMEMATCH_SKIP;
	ch->namematch_default[1] = NAMEMATCH_NONE;
	ch->name_converter = NULL;
	ch->source = -2;
}

void dosnameToDirentry(const struct dos_name_t *dn, struct directory *dir)
{
	strncpy(dir->name, dn->base, 8);
	strncpy(dir->ext, dn->ext, 3);
}
