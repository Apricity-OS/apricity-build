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

#if !defined(EXTENDED__H)

#define EXTENDED__H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

#include <qglobal.h>

class QString;

namespace FS
{
	/** An extended file system.

		A FileSystem for an extended Partition. Of course, extended partitions do not actually have
		a file system, but we need this to be able to create, grow, shrink or move them.

		@author Volker Lanz <vl@fidra.de>
	 */

	class LIBPARTITIONMANAGERPRIVATE_EXPORT extended : public FileSystem
	{
		public:
			extended(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label);

		public:
			static void init() {}

			virtual bool create(Report&, const QString&) const;

			virtual CommandSupportType supportCreate() const { return m_Create; }
			virtual CommandSupportType supportGrow() const { return m_Grow; }
			virtual CommandSupportType supportShrink() const { return m_Shrink; }
			virtual CommandSupportType supportMove() const { return m_Move; }

			virtual bool supportToolFound() const { return true; }

		public:
			static CommandSupportType m_Create;
			static CommandSupportType m_Grow;
			static CommandSupportType m_Shrink;
			static CommandSupportType m_Move;
	};
}

#endif
