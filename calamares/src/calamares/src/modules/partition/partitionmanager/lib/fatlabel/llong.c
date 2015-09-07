/*  Copyright 1999-2003,2006,2008,2009 Alain Knaff.
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

#include "stream.h"
#include "fs.h"
#include "llong.h"
#include "mtools.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

const off_t max_off_t_32 = MAX_OFF_T_B(32); /* Directory */
const off_t max_off_t_seek = MAX_OFF_T_B(SEEK_BITS); /* SCSI */

static int fileTooBig(off_t off)
{
	return (off & ~max_off_t_32) != 0;
}

off_t truncBytes32(off_t off)
{
	if (fileTooBig(off))
	{
		fprintf(stderr, "Internal error, offset too big\n");
		return off; // TODO: this used to be an exit(1)...
	}

	return (off_t) off;
}

off_t sectorsToBytes(struct Stream_t *Stream, off_t off)
{
	DeclareThis(struct Fs_t);
	return (off_t) off << This->sectorShift;
}

int mt_lseek(int fd, off_t where, int whence)
{
	return lseek64(fd, where, whence) >= 0 ? 0 : -1;
}
