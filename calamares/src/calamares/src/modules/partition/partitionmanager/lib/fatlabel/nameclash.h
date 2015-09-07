/*  Copyright 1996-1998,2000-2002,2008,2009 Alain Knaff.
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

#ifndef MTOOLS_NAMECLASH_H
#define MTOOLS_NAMECLASH_H

struct dos_name_t;
struct doscp_t;

typedef enum clash_action
{
	NAMEMATCH_SUCCESS = 0,
	NAMEMATCH_NONE,
	NAMEMATCH_SKIP,
	NAMEMATCH_ERROR,
	NAMEMATCH_GREW
} clash_action;

/* clash handling structure */
struct ClashHandling_t
{
	clash_action action[2];
	clash_action namematch_default[2];

	int nowarn;	/* Don't ask, just do default action if name collision*/
	int got_slots;
	int mod_time;
	char *myname;
	unsigned char *dosname;
	int single;

	int use_longname;
	int ignore_entry;
	int source; /* to prevent the source from overwriting itself */
	int source_entry; /* to account for the space freed up by the original name */
	void (*name_converter)(struct doscp_t *cp, const char *filename, int *mangled, struct dos_name_t *ans);
};


#endif
