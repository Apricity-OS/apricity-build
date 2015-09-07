/*  Copyright 2009 Alain Knaff.
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

#ifndef FILE_NAME_H
#define FILE_NAME_H

#include <wchar.h>
#include <stddef.h>

struct doscp_t;

/**
 * raw dos-name coming straight from the directory entry
 * MYFILE  TXT
 */
struct dos_name_t
{
	char base[8];
	char ext[3];
	char sentinel;
};

struct doscp_t;

int dos_to_wchar(struct doscp_t *fromDos, char *dos, wchar_t *wchar, size_t len);
void wchar_to_dos(struct doscp_t *toDos, wchar_t *wchar, char *dos, size_t len, int *mangled);

struct doscp_t *cp_open(int codepage);
void cp_close(struct doscp_t *cp);

int wchar_to_native(const wchar_t *wchar, char *native, size_t len);
int native_to_wchar(const char *native, wchar_t *wchar, size_t len, const char *end, int *mangled);
wchar_t *unix_name(struct doscp_t *fromDos, const char *base, const char *ext, char Case, wchar_t *answer);

#endif
