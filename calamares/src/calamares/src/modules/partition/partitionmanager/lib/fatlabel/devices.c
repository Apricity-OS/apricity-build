/*  Copyright 1986-1992 Emmet P. Gray.
 *  Copyright 1996-2003,2006,2007,2009 Alain Knaff.
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

#include "mtools.h"
#include "devices.h"

int init_geom(int UNUSED(fd), struct device *dev, struct device *orig_dev, struct stat64 UNUSED(*statbuf))
{
	if(!orig_dev || !orig_dev->tracks || !dev || !dev->tracks)
		return 0; /* no original device. This is ok */

	return(orig_dev->tracks != dev->tracks ||
	       orig_dev->heads != dev->heads ||
	       orig_dev->sectors  != dev->sectors);
}
