/*  Copyright 1996-2005,2007-2009 Alain Knaff.
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

#ifndef MTOOLS_DIRECTORY_H
#define MTOOLS_DIRECTORY_H

#include "vfat.h"

#include <time.h>
#include <stddef.h>

struct dos_name_t;
struct Stream_t;
struct scan_state;
struct ClashHandling_t;

struct directory
{
	char name[8];			/*  0 file name */
	char ext[3];			/*  8 file extension */
	unsigned char attr;		/* 11 attribute byte */
	unsigned char Case;		/* 12 case of short filename */
	unsigned char ctime_ms;		/* 13 creation time, milliseconds (?) */
	unsigned char ctime[2];		/* 14 creation time */
	unsigned char cdate[2];		/* 16 creation date */
	unsigned char adate[2];		/* 18 last access date */
	unsigned char startHi[2];	/* 20 start cluster, Hi */
	unsigned char time[2];		/* 22 time stamp */
	unsigned char date[2];		/* 24 date stamp */
	unsigned char start[2];		/* 26 starting cluster number */
	unsigned char size[4];		/* 28 size of the file */
};

struct direntry_t
{
	struct Stream_t *Dir;
	int entry; /* slot in parent directory (-3 if root) */
	struct directory dir; /* descriptor in parent directory (random if root)*/
	wchar_t name[MAX_VNAMELEN + 1]; /* name in its parent directory, or NULL if root */
	int beginSlot; /* begin and end slot, for delete */
	int endSlot;
};

struct directory *mk_entry(const struct dos_name_t *filename, char attr, unsigned int fat, size_t size, time_t date, struct directory *ndir);
struct directory *mk_entry_from_base(const char *base, char attr, unsigned int fat, size_t size, time_t date, struct directory *ndir);

int dir_grow(struct Stream_t *Dir, int size);

void bufferize(struct Stream_t **Dir);

struct directory *dir_read(struct direntry_t *entry, int *error);

void initializeDirentry(struct direntry_t *entry, struct Stream_t *Dir);
int isNotFound(struct direntry_t *entry);
void low_level_dir_write(struct direntry_t *entry);
void dosnameToDirentry(const struct dos_name_t *n, struct directory *dir);

int lookupForInsert(struct Stream_t *Dir, struct direntry_t *direntry, struct dos_name_t *dosname, char *longname, struct scan_state *ssp, int ignore_entry, int source_entry, int pessimisticShortRename, int use_longname);

typedef int (write_data_callback)(struct dos_name_t*, struct direntry_t*);

void init_clash_handling(struct ClashHandling_t *ch);
int mwrite_one(struct Stream_t *Dir, const char *argname, write_data_callback *cb, struct ClashHandling_t *ch);

#endif


