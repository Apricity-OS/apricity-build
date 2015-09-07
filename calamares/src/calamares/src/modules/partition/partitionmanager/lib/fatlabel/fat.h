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

#ifndef MTOOLS_FAT_H
#define MTOOLS_FAT_H

#include <stddef.h>

struct Stream_t;
struct directory;

int getfreeMinClusters(struct Stream_t *Stream, size_t ref);
unsigned int getStart(struct Stream_t *Dir, struct directory *dir);

#endif
