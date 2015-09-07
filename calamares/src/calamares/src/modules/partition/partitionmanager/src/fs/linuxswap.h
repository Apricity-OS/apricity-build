/***************************************************************************
 *   Copyright (C) 2008,2009 by Volker Lanz <vl@fidra.de>                  *
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

#if !defined(LINUXSWAP__H)

#define LINUXSWAP__H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

#include <qglobal.h>

class Report;

class QString;

namespace FS
{
	/** A linux swap pseudo file system.
		@author Volker Lanz <vl@fidra.de>
	 */
	class LIBPARTITIONMANAGERPRIVATE_EXPORT linuxswap : public FileSystem
	{
		public:
			linuxswap(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label);

		public:
			static void init();

			virtual bool create(Report& report, const QString& deviceNode) const;
			virtual bool resize(Report& report, const QString& deviceNode, qint64 length) const;
			virtual bool writeLabel(Report& report, const QString& deviceNode, const QString& newLabel);
			virtual bool copy(Report& report, const QString& targetDeviceNode, const QString& sourceDeviceNode) const;
			virtual bool updateUUID(Report& report, const QString& deviceNode) const;

			virtual bool canMount(const QString&) const { return true; }
			virtual bool canUnmount(const QString&) const { return true; }

			virtual bool mount(const QString& deviceNode);
			virtual bool unmount(const QString& deviceNode);

			virtual QString mountTitle() const;
			virtual QString unmountTitle() const;

			virtual CommandSupportType supportCreate() const { return m_Create; }
			virtual CommandSupportType supportGrow() const { return m_Grow; }
			virtual CommandSupportType supportShrink() const { return m_Shrink; }
			virtual CommandSupportType supportMove() const { return m_Move; }
			virtual CommandSupportType supportCopy() const { return m_Copy; }
			virtual CommandSupportType supportGetLabel() const { return m_GetLabel; }
			virtual CommandSupportType supportSetLabel() const { return m_SetLabel; }
			virtual CommandSupportType supportUpdateUUID() const { return m_UpdateUUID; }
			virtual CommandSupportType supportGetUUID() const { return m_GetUUID; }

			virtual qint64 maxLabelLength() const;
			virtual SupportTool supportToolName() const;
			virtual bool supportToolFound() const;

		public:
			static CommandSupportType m_Create;
			static CommandSupportType m_Grow;
			static CommandSupportType m_Shrink;
			static CommandSupportType m_Move;
			static CommandSupportType m_Copy;
			static CommandSupportType m_SetLabel;
			static CommandSupportType m_GetLabel;
			static CommandSupportType m_UpdateUUID;
			static CommandSupportType m_GetUUID;
	};
}

#endif
