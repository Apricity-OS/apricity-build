/*  Copyright 1999,2001-2004,2007-2009 Alain Knaff.
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

#ifndef MTOOLS_LLONG_H
#define MTOOLS_LLONG_H

#include <stddef.h>
#include <sys/types.h>

#define min(a,b) ((a) < (b) ? (a) : (b))
#define MAX_OFF_T_B(bits) ((((off_t) 1 << min(bits-1, sizeof(off_t)*8 - 2)) -1) << 1 | 1)

#define SEEK_BITS 63

struct Stream_t;

extern const off_t max_off_t_32;
extern const off_t max_off_t_seek;

off_t truncBytes32(off_t off);
off_t sectorsToBytes(struct Stream_t *This, off_t off);
int mt_lseek(int fd, off_t where, int whence);

#endif
