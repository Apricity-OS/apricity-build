/*  Copyright 1995 David C. Niemi
 *  Copyright 1996-1998,2000-2003,2005,2007-2009 Alain Knaff.
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

#ifndef MTOOLS_VFAT_H
#define MTOOLS_VFAT_H

#include <wchar.h>

struct dos_name_t;
struct direntry_t;

struct unicode_char
{
	unsigned char lchar;
	unsigned char uchar;
};

#define MAX_VFAT_SUBENTRIES 20		/* Max useful # of VSEs */
#define VSE_NAMELEN 13

#define VSE1SIZE 5
#define VSE2SIZE 6
#define VSE3SIZE 2

#include "stream.h"

struct vfat_subentry
{
	unsigned char id;		/* 0x40 = last; & 0x1f = VSE ID */
	struct unicode_char text1[VSE1SIZE];
	unsigned char attribute;	/* 0x0f for VFAT */
	unsigned char hash1;		/* Always 0? */
	unsigned char sum;		/* Checksum of short name */
	struct unicode_char text2[VSE2SIZE];
	unsigned char sector_l;		/* 0 for VFAT */
	unsigned char sector_u;		/* 0 for VFAT */
	struct unicode_char text3[VSE3SIZE];
};

/* Enough size for a worst case number of full VSEs plus a null */
#define VBUFSIZE ((MAX_VFAT_SUBENTRIES*VSE_NAMELEN) + 1)

/* Max legal length of a VFAT long name */
#define MAX_VNAMELEN 255

#define VSE_PRESENT 0x01
#define VSE_LAST 0x40
#define VSE_MASK 0x1f

struct vfat_state
{
	wchar_t name[VBUFSIZE];
	int status; /* is now a bit map of 32 bits */
	int subentries;
	unsigned char sum; /* no need to remember the sum for each entry, it is the same anyways */
	int present;
};

struct scan_state
{
	int match_free;
	int shortmatch;
	int longmatch;
	unsigned int free_start;
	unsigned int free_end;
	int slot;
	int got_slots;
	unsigned int size_needed;
	int max_entry;
};

int unicode_write(wchar_t *, struct unicode_char *, int num, int *end);
int clear_vses(struct Stream_t *, int, size_t);
void autorename_short(struct dos_name_t *, int);
void autorename_long(char *, int);
int vfat_lookup(struct direntry_t *entry, const char *filename, int length, int flags, char *shortname, char *longname);
int dir_write(struct direntry_t *entry);
int write_vfat(struct Stream_t *, struct dos_name_t *, char *, int, struct direntry_t *);

#define DO_OPEN 1 /* open all files that are found */
#define ACCEPT_LABEL 0x08
#define ACCEPT_DIR 0x10
#define ACCEPT_PLAIN 0x20
#define MATCH_ANY 0x40
#define NO_MSG 0x80
#define NO_DOTS 0x100 /* accept no dots if matched by wildcard */

#endif
