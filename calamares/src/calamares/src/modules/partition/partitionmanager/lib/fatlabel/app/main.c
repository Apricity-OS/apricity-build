/***************************************************************************
 *   Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "fatlabel/fatlabel.h"

#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		fprintf(stderr, "usage: %s <device> <label>\n", argv[0]);
		return 1;
	}

	const char* deviceName = argv[1];
	const char* newLabel = argv[2];

	int ret = fatlabel_set_label(deviceName, newLabel);

	switch(ret)
	{
		case -1:
			fprintf(stderr, "Label too long\n");
			break;

		case -2:
			fprintf(stderr, "%s: Cannot initialize drive\n", argv[0]);
			break;

		case -3:
			fprintf(stderr, "vfat lookup failed\n");
			break;

		case 0:
		default:
			break;
	}

	return ret;
}
