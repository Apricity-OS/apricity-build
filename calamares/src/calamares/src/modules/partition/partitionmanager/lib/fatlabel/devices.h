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

#ifndef MTOOLS_DEVICES_H
#define MTOOLS_DEVICES_H

#include <sys/types.h>
#include <sys/stat.h>

struct device
{
	const char *name;       /* full path to device */
	int fat_bits;			/* FAT encoding scheme */
	unsigned int mode;		/* any special open() flags */
	unsigned int tracks;	/* tracks */
	unsigned int heads;		/* heads */
	unsigned int sectors;	/* sectors */
	unsigned int hidden;	/* number of hidden sectors. Used for mformatting partitioned devices */
	off_t offset;	       	/* skip this many bytes */
	unsigned int partition;
	unsigned int misc_flags;

	/* Linux only stuff */
	unsigned int ssize;
	unsigned int use_2m;

	/* internal variables */
	int file_nr;		/* used during parsing */
	unsigned int blocksize;	  /* size of disk block in bytes */
	int codepage; /* codepage for shortname encoding */
	const char *cfg_filename; /* used for debugging purposes */
};

int init_geom(int fd, struct device *dev, struct device *orig_dev, struct stat64 *statbuf);

#endif
