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

#include "fs/lvm2_pv.h"

#include "util/externalcommand.h"
#include "util/capacity.h"

#include <QString>

namespace FS
{
	FileSystem::CommandSupportType lvm2_pv::m_GetUsed = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType lvm2_pv::m_GetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType lvm2_pv::m_Create = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType lvm2_pv::m_Grow = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType lvm2_pv::m_Shrink = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType lvm2_pv::m_Move = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType lvm2_pv::m_Check = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType lvm2_pv::m_Copy = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType lvm2_pv::m_Backup = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType lvm2_pv::m_SetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType lvm2_pv::m_UpdateUUID = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType lvm2_pv::m_GetUUID = FileSystem::cmdSupportNone;

	lvm2_pv::lvm2_pv(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Lvm2_PV)
	{
	}

	void lvm2_pv::init()
	{
		m_Create = findExternal(QStringLiteral("lvm")) ? cmdSupportFileSystem : cmdSupportNone;
		m_Check = findExternal(QStringLiteral("lvm")) ? cmdSupportFileSystem : cmdSupportNone;

		m_GetLabel = cmdSupportCore;
		m_UpdateUUID = findExternal(QStringLiteral("lvm")) ? cmdSupportFileSystem : cmdSupportNone;

		m_Copy = cmdSupportNone; // Copying PV can confuse LVM
		m_Move = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;

		m_GetLabel = cmdSupportNone;
		m_Backup = cmdSupportCore;
		m_GetUUID = cmdSupportCore;
	}

	bool lvm2_pv::supportToolFound() const
	{
		return
// 			m_GetUsed != cmdSupportNone &&
// 			m_GetLabel != cmdSupportNone &&
// 			m_SetLabel != cmdSupportNone &&
			m_Create != cmdSupportNone &&
			m_Check != cmdSupportNone &&
			m_UpdateUUID != cmdSupportNone &&
// 			m_Grow != cmdSupportNone &&
// 			m_Shrink != cmdSupportNone &&
// 			m_Copy != cmdSupportNone &&
			m_Move != cmdSupportNone &&
			m_Backup != cmdSupportNone &&
			m_GetUUID != cmdSupportNone;
	}

	FileSystem::SupportTool lvm2_pv::supportToolName() const
	{
		return SupportTool(QStringLiteral("lvm2"), QUrl(QStringLiteral("http://sourceware.org/lvm2/")));
	}

	qint64 lvm2_pv::maxCapacity() const
	{
		 return Capacity::unitFactor(Capacity::Byte, Capacity::EiB);
	}

	bool lvm2_pv::check(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("lvm"), QStringList() << QStringLiteral("pvck") << QStringLiteral("-v") << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool lvm2_pv::create(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("lvm"), QStringList() << QStringLiteral("pvcreate") << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool lvm2_pv::remove(Report& report, const QString& deviceNode) const
	{
// 		TODO: check if PV is a member of an exported VG
		ExternalCommand cmd(report, QStringLiteral("lvm"), QStringList() << QStringLiteral("pvremove") << QStringLiteral("-ffy") << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool lvm2_pv::updateUUID(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("lvm"), QStringList() << QStringLiteral("pvchange") << QStringLiteral("-u") << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}
}
