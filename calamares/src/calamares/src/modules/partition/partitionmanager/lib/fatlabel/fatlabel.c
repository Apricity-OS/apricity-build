/*  Copyright 1986-1992 Emmet P. Gray.
 *  Copyright 1996-1998,2000-2002,2005,2007-2009 Alain Knaff.
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
 * mlabel.c
 * Make an MSDOS volume label
 */

#include "fatlabel.h"

#include "msdos.h"
#include "nameclash.h"
#include "init.h"
#include "file.h"
#include "directory.h"
#include "force_io.h"
#include "file_name.h"

#include <wctype.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

/**
 * Wipe the given entry
 */
static void wipeEntry(struct direntry_t *entry)
{
	struct direntry_t longNameEntry;

	initializeDirentry(&longNameEntry, entry->Dir);

	for(int i = entry->beginSlot; i< entry->endSlot; i++)
	{
	    int error;
	    longNameEntry.entry=i;
	    dir_read(&longNameEntry, &error);

	    if(error)
			break;
	    longNameEntry.dir.name[0] = (char) DELMARK;
	    dir_write(&longNameEntry);
	}

	entry->dir.name[0] = (char) DELMARK;
	dir_write(entry);
}

void label_name(struct doscp_t *cp, const char *filename, int *mangled, struct dos_name_t *ans)
{
	memset(ans, ' ', 8 + 3);
	ans->sentinel = '\0';

	wchar_t wbuffer[12];
	int len = native_to_wchar(filename, wbuffer, 11, 0, 0);

	if (len > 11)
	{
		*mangled = 1;
		len = 11;
	}
	else
		*mangled = 0;

	int have_lower = 0;
	int have_upper = 0;
	int i = 0;

	for(i = 0; i < len; i++)
	{
		if (islower(wbuffer[i]))
			have_lower = 1;

		if (isupper(wbuffer[i]))
			have_upper = 1;

		wbuffer[i] = towupper(wbuffer[i]);

		if (wcschr(L"^+=/[]:,?*\\<>|\".", wbuffer[i]))
		{
			*mangled = 1;
			wbuffer[i] = '~';
		}
	}

	if (have_lower && have_upper)
		*mangled = 1;

	wchar_to_dos(cp, wbuffer, ans->base, len, mangled);
}

int labelit(struct dos_name_t* dosname, struct direntry_t* entry)
{
	time_t now = time(NULL);
	mk_entry(dosname, 0x8, 0, 0, now, &entry->dir);
	return 0;
}

int fatlabel_set_label(const char* device_name, const char* new_label)
{
	if (strlen(new_label) > VBUFSIZE)
		return -1;

	/*
	 * 1. Init clash handling
	 */
	struct ClashHandling_t ch;
	init_clash_handling(&ch);
	ch.name_converter = label_name;
	ch.ignore_entry = -2;

	/*
	 * 2. Open root dir
	 */
	struct Stream_t* RootDir = fs_init(device_name, O_RDWR);

	if (RootDir)
		RootDir = OpenRoot(RootDir);

	if (!RootDir)
	{
		fprintf(stderr, "Opening root dir failed.\n");
		return -2;
	}

	/*
	 * 3. Init dir entry
	 */
	struct direntry_t entry;
	initializeDirentry(&entry, RootDir);

	/*
	 * 4. Lookup vfat
	 */
	char longname[VBUFSIZE];
	char shortname[45];
	if (vfat_lookup(&entry, 0, 0, ACCEPT_LABEL | MATCH_ANY, shortname, longname) == -2)
	{
		fprintf(stderr, "Looking up vfat failed.\n");
		free_stream(&RootDir);
		return -3;
	}

	/*
	 * 5. Wipe existing entry.
	 */
	if (!isNotFound(&entry))
	{
		/* if we have a label, wipe it out before putting new one */
		entry.dir.attr = 0; /* for old mlabel */
		wipeEntry(&entry);
	}

	/*
	 * 6. Write new entry
	 */
	ch.ignore_entry = 1;

	/* don't try to write the label if it's empty */
	int result = strlen(new_label) ? mwrite_one(RootDir, new_label, labelit, &ch) : 0;

	/*
	 * 7. Load FAT boot record
	 */
	union bootsector boot;
	struct Stream_t* Fs = GetFs(RootDir);
	int have_boot = force_read(Fs, boot.characters, 0, sizeof(boot)) == sizeof(boot);

	struct label_blk_t* labelBlock = WORD_S(fatlen) ? &boot.boot.ext.old.labelBlock : &boot.boot.ext.fat32.labelBlock;

	/*
	 * 8. Get "dosconvert" struct
	 */
	struct dos_name_t dosname;
	struct doscp_t* cp = GET_DOSCONVERT(Fs);

	/*
	 * 9. Convert label
	 */
	int mangled = 0;
	label_name(cp, new_label, &mangled, &dosname);

	/*
	 * 10. Overwrite FAT boot record
	 */
	if (have_boot && boot.boot.descr >= 0xf0 && labelBlock->dos4 == 0x29)
	{
		strncpy(labelBlock->label, dosname.base, 11);
		force_write(Fs, (char *)&boot, 0, sizeof(boot));
	}

	free_stream(&RootDir);
	fs_close(Fs);

	return result;
}
