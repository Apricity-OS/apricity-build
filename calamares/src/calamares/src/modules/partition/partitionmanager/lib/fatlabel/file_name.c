/*  Copyright 1995 David C. Niemi
 *  Copyright 1996-1998,2000-2002,2008,2009 Alain Knaff.
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

#include "msdos.h"
#include "mtools.h"
#include "vfat.h"
#include "file_name.h"

#include <string.h>
#include <ctype.h>

typedef enum Case_l {
	NONE,
	UPPER,
	LOWER
} Case_t;


/*
 * Get rid of spaces in an MSDOS 'raw' name (one that has come from the
 * directory structure) so that it can be used for regular expression
 * matching with a Unix filename.  Also used to 'unfix' a name that has
 * been altered by dos_name().
 */

wchar_t *unix_name(struct doscp_t *dosCp, const char *base, const char *ext, char Case, wchar_t *ret)
{
	char tname[9];

	strncpy(tname, base, 8);
	tname[8] = '\0';

	char *s;
	if ((s = strchr(tname, ' ')))
		*s = '\0';

	int i;
	if(Case & BASECASE)
		for(i = 0;i < 8 && tname[i]; i++)
			tname[i] = tolower(tname[i]);

	char text[4];
	strncpy(text, ext, 3);
	text[3] = '\0';
	if ((s = strchr(text, ' ')))
		*s = '\0';

	if(Case & EXTCASE)
		for(i = 0; i < 3 && text[i]; i++)
			text[i] = tolower(text[i]);

	char ans[13];
	if (*text)
	{
		strcpy(ans, tname);
		strcat(ans, ".");
		strcat(ans, text);
	}
	else
		strcpy(ans, tname);

	/* fix special characters (above 0x80) */
	dos_to_wchar(dosCp, ans, ret, 12);

	return ret;
}

/* If null encountered, set *end to 0x40 and write nulls rest of way
 * 950820: Win95 does not like this!  It complains about bad characters.
 * So, instead: If null encountered, set *end to 0x40, write the null, and
 * write 0xff the rest of the way (that is what Win95 seems to do; hopefully
 * that will make it happy)
 */
/* Always return num */
int unicode_write(wchar_t *in, struct unicode_char *out, int num, int *end_p)
{
	int j;

	for (j = 0; j < num; ++j)
	{
		if (*end_p) /* Fill with 0xff */
			out->uchar = out->lchar = (char) 0xff;
		else
		{
			out->uchar = *in >> 8;
			out->lchar = *in;
			if (! *in)
				*end_p = VSE_LAST;
		}

		++out;
		++in;
	}

	return num;
}
