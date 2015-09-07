/***************************************************************************
 *   Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                       *
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

#if !defined(FAT32__H)

#define FAT32__H

#include "util/libpartitionmanagerexport.h"

#include "fs/fat16.h"

#include <qglobal.h>

class Report;

class QString;

namespace FS
{
	/** A fat32 file system.

		Basically the same as a fat16 file system.

		@author Volker Lanz <vl@fidra.de>
	 */
	class LIBPARTITIONMANAGERPRIVATE_EXPORT fat32 : public fat16
	{
		public:
			fat32(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label);

		public:
			static void init() {}

			virtual bool create(Report& report, const QString& deviceNode) const;
			virtual bool updateUUID(Report& report, const QString& deviceNode) const;

			virtual qint64 minCapacity() const;
			virtual qint64 maxCapacity() const;
	};
}

#endif
