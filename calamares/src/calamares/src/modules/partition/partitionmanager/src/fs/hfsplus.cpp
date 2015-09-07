/***************************************************************************
 *   Copyright (C) 2008,2011 by Volker Lanz <vl@fidra.de>                  *
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

#include "fs/hfsplus.h"

#include "util/externalcommand.h"
#include "util/capacity.h"

#include <QStringList>

namespace FS
{
	FileSystem::CommandSupportType hfsplus::m_GetUsed = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType hfsplus::m_Shrink = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType hfsplus::m_Move = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType hfsplus::m_Check = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType hfsplus::m_Copy = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType hfsplus::m_Backup = FileSystem::cmdSupportNone;

	hfsplus::hfsplus(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::HfsPlus)
	{
	}

	void hfsplus::init()
	{
		m_Check = findExternal(QStringLiteral("hpfsck")) ? cmdSupportFileSystem : cmdSupportNone;
		m_Copy = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
		m_Move = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
		m_Backup = cmdSupportCore;
	}

	bool hfsplus::supportToolFound() const
	{
		return
//			m_GetUsed != cmdSupportNone &&
// 			m_GetLabel != cmdSupportNone &&
// 			m_SetLabel != cmdSupportNone &&
// 			m_Create != cmdSupportNone &&
			m_Check != cmdSupportNone &&
// 			m_UpdateUUID != cmdSupportNone &&
// 			m_Grow != cmdSupportNone &&
// 			m_Shrink != cmdSupportNone &&
			m_Copy != cmdSupportNone &&
			m_Move != cmdSupportNone &&
			m_Backup != cmdSupportNone;
// 			m_GetUUID != cmdSupportNone;
	}

	FileSystem::SupportTool hfsplus::supportToolName() const
	{
		return SupportTool(QStringLiteral("hfsplus"), QUrl());
	}

	qint64 hfsplus::maxCapacity() const
	{
		return 8 * Capacity::unitFactor(Capacity::Byte, Capacity::EiB);
	}

	qint64 hfsplus::maxLabelLength() const
	{
		 return 63;
	}

	bool hfsplus::check(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("hpfsck"), QStringList() << QStringLiteral("-v") << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}
}
