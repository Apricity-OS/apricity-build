/*  Copyright 1996-1999,2001,2002,2005,2006,2008,2009 Alain Knaff.
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


#ifndef MTOOLS_FORCE_IO_H
#define MTOOLS_FORCE_IO_H

#include <sys/types.h>
#include <stddef.h>

struct Stream_t;

int force_write(struct Stream_t *Stream, char *buf, off_t start, size_t len);
int force_read(struct Stream_t *Stream, char *buf, off_t start, size_t len);

#endif
