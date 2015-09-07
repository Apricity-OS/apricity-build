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

#ifndef MTOOLS_MTOOLS_H
#define MTOOLS_MTOOLS_H

#define UNUSED(x) x __attribute__ ((unused))
#define New(type) ((type*)(calloc(1,sizeof(type))))
#define Grow(adr,n,type) ((type*)(realloc((char *)adr,n*sizeof(type))))
#define NewArray(size,type) ((type*)(calloc((size),sizeof(type))))
#define maximize(target, max) do { \
	if(max < 0) \
	{ \
		if(target > 0) \
			target = 0; \
	} \
	else if(target > max) \
		target = max; \
} while(0)

#endif
