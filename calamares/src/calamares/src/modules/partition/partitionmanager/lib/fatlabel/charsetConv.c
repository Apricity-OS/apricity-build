/*  Copyright 2008,2009 Alain Knaff.
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
 *
 * Various character set conversions used by mtools
 */
#include "msdos.h"
#include "mtools.h"
#include "file_name.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <iconv.h>
#include <langinfo.h>
#include <string.h>

struct doscp_t
{
	iconv_t from;
	iconv_t to;
} doscp_t;

static unsigned int mtools_default_codepage = 850;
static char *wcharCp = NULL;

static char* wcharTries[] =
{
	"WCHAR_T",
	"UTF-32BE", "UTF-32LE",
	"UTF-16BE", "UTF-16LE",
	"UTF-32", "UTF-16",
	"UCS-4BE", "UCS-4LE",
	"UCS-2BE", "UCS-2LE",
	"UCS-4", "UCS-2"
};

static wchar_t *testString = L"ab";

static int tryConvert(char *testCp)
{
	char *inbuf = (char *)testString;
	size_t inbufLen = 2*sizeof(wchar_t);
	char outbuf[3];
	char *outbufP = outbuf;
	size_t outbufLen = 2*sizeof(char);
	iconv_t test = iconv_open("ASCII", testCp);

	if(test == (iconv_t) -1)
		goto fail0;

	size_t res = iconv(test, &inbuf, &inbufLen, &outbufP, &outbufLen);

	if(res != 0 || outbufLen != 0 || inbufLen != 0)
		goto fail;

	if(memcmp(outbuf, "ab", 2))
		goto fail;

	return 1;

fail:
	iconv_close(test);

fail0:
	return 0;
}

static const char *getWcharCp()
{
	if(wcharCp != NULL)
		return wcharCp;

	unsigned int i;
	for(i = 0; i < sizeof(wcharTries) / sizeof(wcharTries[0]); i++)
		if(tryConvert(wcharTries[i]))
			return wcharCp = wcharTries[i];

	fprintf(stderr, "No codepage found for wchar_t\n");

	return NULL;
}

struct doscp_t *cp_open(int codepage)
{
	char dosCp[17];

	if(codepage == 0)
		codepage = mtools_default_codepage;

	if(codepage < 0 || codepage > 9999)
	{
		fprintf(stderr, "Bad codepage %d\n", codepage);
		return NULL;
	}

	if(getWcharCp() == NULL)
		return NULL;

	sprintf(dosCp, "CP%d", codepage);

	iconv_t *from = iconv_open(wcharCp, dosCp);

	if(from == (iconv_t)-1)
	{
		fprintf(stderr, "Error converting to codepage %d %s\n", codepage, strerror(errno));
		return NULL;
	}

	sprintf(dosCp, "CP%d//TRANSLIT", codepage);
	iconv_t *to = iconv_open(dosCp, wcharCp);
	if(to == (iconv_t)-1)
	{
		/* Transliteration not supported? */
		sprintf(dosCp, "CP%d", codepage);
		to = iconv_open(dosCp, wcharCp);
	}

	if(to == (iconv_t)-1)
	{
		iconv_close(from);
		fprintf(stderr, "Error converting to codepage %d %s\n", codepage, strerror(errno));
		return NULL;
	}

	struct doscp_t *ret = New(struct doscp_t);

	if(ret == NULL)
		return ret;

	ret->from = from;
	ret->to = to;

	return ret;
}

void cp_close(struct doscp_t *cp)
{
	iconv_close(cp->to);
	iconv_close(cp->from);
	free(cp);
}

int dos_to_wchar(struct doscp_t *cp, char *dos, wchar_t *wchar, size_t len)
{
	size_t in_len = len;
	size_t out_len = len*sizeof(wchar_t);

	wchar_t *dptr = wchar;

	int r = iconv(cp->from, &dos, &in_len, (char **)&dptr, &out_len);

	if(r < 0)
		return r;

	*dptr = L'\0';

	return dptr - wchar;
}

/**
 * Converts len wide character to destination. Caller's responsibility to
 * ensure that dest is large enough.
 * mangled will be set if there has been an untranslatable character.
 */
static int safe_iconv(iconv_t conv, const wchar_t *wchar, char *dest, size_t len, int *mangled)
{
	size_t in_len = len * sizeof(wchar_t);
	size_t out_len = len * 4;
	char *dptr = dest;

	while (in_len > 0)
	{
		int r = iconv(conv, (char**)&wchar, &in_len, &dptr, &out_len);

		/* everything transformed, or error that is _not_ a bad character */
		if(r >= 0 || errno != EILSEQ)
			break;

		*mangled |= 1;

		if(dptr)
			*dptr++ = '_';

		in_len--;

		wchar++;
		out_len--;
	}

	len = dptr-dest; /* how many dest characters have there been generated */

	/* eliminate question marks which might have been formed by
	   untransliterable characters */
	unsigned int i;
	for (i = 0; i < len; i++)
	{
		if (dest[i] == '?')
		{
			dest[i] = '_';
			*mangled |= 1;
		}
	}

	return len;
}

void wchar_to_dos(struct doscp_t *cp, wchar_t *wchar, char *dos, size_t len, int *mangled)
{
	safe_iconv(cp->to, wchar, dos, len, mangled);
}

static iconv_t to_native = NULL;

static int initialize_to_native(void)
{
	if(to_native != NULL)
		return 0;

	char *li = nl_langinfo(CODESET);
	int len = strlen(li) + 11;

	if(getWcharCp() == NULL)
		return -1; // TODO: make caller check return code

	char *cp = malloc(len);
	strcpy(cp, li);
	strcat(cp, "//TRANSLIT");

	to_native = iconv_open(cp, wcharCp);

	if(to_native == (iconv_t) -1)
		to_native = iconv_open(li, wcharCp);

	if(to_native == (iconv_t) -1)
		fprintf(stderr, "Could not allocate iconv for %s\n", cp);

	free(cp);

	if(to_native == (iconv_t) -1)
		return -1;

	return 0;
}


/**
 * Convert wchar string to native, converting at most len wchar characters
 * Returns number of generated native characters
 */
int wchar_to_native(const wchar_t *wchar, char *native, size_t len)
{
	int mangled;

	initialize_to_native();

	len = wcsnlen(wchar,len);

	int r = safe_iconv(to_native, wchar, native, len, &mangled);

	native[r] = '\0';

	return r;
}

/**
 * Convert native string to wchar string, converting at most len wchar
 * characters. If end is supplied, stop conversion when source pointer
 * exceeds end. Returns number of converted wchars
 */
int native_to_wchar(const char *native, wchar_t *wchar, size_t len, const char *end, int *mangled)
{
	mbstate_t ps;

	memset(&ps, 0, sizeof(ps));

	unsigned int i;
	for(i = 0; i < len && (native < end || !end); i++)
	{
		int r = mbrtowc(wchar+i, native, len, &ps);

		if(r < 0)
		{
			/* Unconvertible character. Just pretend it's Latin1
			   encoded (if valid Latin1 character) or substitue
			   with an underscore if not
			*/

			char c = *native;
			if(c >= '\xa0' && c < '\xff')
				wchar[i] = c & 0xff;
			else
				wchar[i] = '_';

			memset(&ps, 0, sizeof(ps));

			r = 1;
		}

		if(r == 0)
			break;

		native += r;
	}

	if(mangled && end && native < end)
		*mangled |= 3;

	wchar[i]='\0';

	return (int)i;
}
