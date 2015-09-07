/*  Copyright 1996,1997,1999,2001,2002,2009 Alain Knaff.
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

#ifndef MTOOLS_PLAINIO_H
#define MTOOLS_PLAINIO_H

#include <stddef.h>

struct device;
struct Stream_t;

#define NO_PRIV 1
#define NO_OFFSET 2

struct Stream_t *SimpleFileOpen(struct device *dev, struct device *orig_dev, const char *name, int mode, char *errmsg, int mode2, int locked, size_t *maxSize);
int SimpleFileClose(struct Stream_t* Stream);

#endif
