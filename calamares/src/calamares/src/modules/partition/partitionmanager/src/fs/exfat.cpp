/***************************************************************************
 *   Copyright (C) 2012 by Volker Lanz <vl@fidra.de>                       *
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

#include "fs/exfat.h"

#include "util/externalcommand.h"
#include "util/capacity.h"

#include <QString>
#include <QRegExp>

namespace FS
{
	FileSystem::CommandSupportType exfat::m_GetUsed = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType exfat::m_GetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType exfat::m_Create = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType exfat::m_Grow = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType exfat::m_Shrink = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType exfat::m_Move = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType exfat::m_Check = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType exfat::m_Copy = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType exfat::m_Backup = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType exfat::m_SetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType exfat::m_UpdateUUID = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType exfat::m_GetUUID = FileSystem::cmdSupportNone;

	exfat::exfat(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Exfat)
	{
	}

	void exfat::init()
	{
		m_Create = findExternal(QStringLiteral("mkfs.exfat")) ? cmdSupportFileSystem : cmdSupportNone;
		m_Check = findExternal(QStringLiteral("exfatfsck"), QStringList(), 1) ? cmdSupportFileSystem : cmdSupportNone;

		m_GetLabel = cmdSupportCore;
		m_SetLabel = findExternal(QStringLiteral("exfatlabel")) ? cmdSupportFileSystem : cmdSupportNone;
		m_UpdateUUID = cmdSupportNone;

		m_Copy = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
		m_Move = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;

		m_GetLabel = cmdSupportCore;
		m_Backup = cmdSupportCore;
		m_GetUUID = cmdSupportCore;
	}

	bool exfat::supportToolFound() const
	{
		return
// 			m_GetUsed != cmdSupportNone &&
			m_GetLabel != cmdSupportNone &&
			m_SetLabel != cmdSupportNone &&
			m_Create != cmdSupportNone &&
			m_Check != cmdSupportNone &&
// 			m_UpdateUUID != cmdSupportNone &&
// 			m_Grow != cmdSupportNone &&
// 			m_Shrink != cmdSupportNone &&
			m_Copy != cmdSupportNone &&
			m_Move != cmdSupportNone &&
			m_Backup != cmdSupportNone &&
			m_GetUUID != cmdSupportNone;
	}

	FileSystem::SupportTool exfat::supportToolName() const
	{
		return SupportTool(QStringLiteral("exfat-utils"), QUrl(QStringLiteral("http://code.google.com/p/exfat/")));
	}

	qint64 exfat::maxCapacity() const
	{
		 return Capacity::unitFactor(Capacity::Byte, Capacity::EiB);
	}

	qint64 exfat::maxLabelLength() const
	{
		 return 15;
	}

	bool exfat::check(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("exfatfsck"), QStringList() << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool exfat::create(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("mkfs.exfat"), QStringList() << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool exfat::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
	{
		ExternalCommand cmd(report, QStringLiteral("exfatlabel"), QStringList() << deviceNode << newLabel);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool exfat::updateUUID(Report& report, const QString& deviceNode) const
	{
		Q_UNUSED(report);
		Q_UNUSED(deviceNode);

		return false;
	}
}
