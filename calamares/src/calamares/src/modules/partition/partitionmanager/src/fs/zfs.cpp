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

#include "fs/zfs.h"

#include "util/externalcommand.h"
#include "util/capacity.h"
#include "util/report.h"

#include <QString>

namespace FS
{
	FileSystem::CommandSupportType zfs::m_GetUsed = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType zfs::m_GetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType zfs::m_Create = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType zfs::m_Grow = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType zfs::m_Shrink = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType zfs::m_Move = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType zfs::m_Check = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType zfs::m_Copy = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType zfs::m_Backup = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType zfs::m_SetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType zfs::m_UpdateUUID = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType zfs::m_GetUUID = FileSystem::cmdSupportNone;

	zfs::zfs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Zfs)
	{
	}

	void zfs::init()
	{
		m_SetLabel = findExternal(QStringLiteral("zpool"), QStringList(), 2) ? cmdSupportFileSystem : cmdSupportNone;

		m_GetLabel = cmdSupportCore;
		m_Backup = cmdSupportCore;
		m_GetUUID = cmdSupportCore;
	}

	bool zfs::supportToolFound() const
	{
		return
// 			m_GetUsed != cmdSupportNone &&
			m_GetLabel != cmdSupportNone &&
			m_SetLabel != cmdSupportNone &&
// 			m_Create != cmdSupportNone &&
// 			m_Check != cmdSupportNone &&
// 			m_UpdateUUID != cmdSupportNone &&
// 			m_Grow != cmdSupportNone &&
// 			m_Shrink != cmdSupportNone &&
// 			m_Copy != cmdSupportNone &&
// 			m_Move != cmdSupportNone &&
			m_Backup != cmdSupportNone &&
			m_GetUUID != cmdSupportNone;
	}

	FileSystem::SupportTool zfs::supportToolName() const
	{
		return SupportTool(QStringLiteral("zfs"), QUrl(QStringLiteral("http://zfsonlinux.org/")));
	}

	qint64 zfs::minCapacity() const
	{
		 return 64 * Capacity::unitFactor(Capacity::Byte, Capacity::MiB);
	}

	qint64 zfs::maxCapacity() const
	{
		 return Capacity::unitFactor(Capacity::Byte, Capacity::EiB);
	}

	bool zfs::remove(Report& report, const QString& deviceNode) const
	{
		Q_UNUSED(deviceNode)
// 		TODO: check if -f option is needed
		ExternalCommand cmd(report, QStringLiteral("zpool"), QStringList() << QStringLiteral("destroy") << QStringLiteral("-f") << this->label());
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool zfs::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
	{
		Q_UNUSED(deviceNode)
		ExternalCommand cmd1(report, QStringLiteral("zpool"), QStringList() << QStringLiteral("export") << this->label());
		ExternalCommand cmd2(report, QStringLiteral("zpool"), QStringList() << QStringLiteral("import") << this->label() << newLabel);
		return cmd1.run(-1) && cmd1.exitCode() == 0 && cmd2.run(-1) && cmd2.exitCode() == 0;
	}
}
