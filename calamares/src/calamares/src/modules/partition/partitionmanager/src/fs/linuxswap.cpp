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

#include "fs/linuxswap.h"

#include "util/externalcommand.h"

#include <KLocalizedString>

namespace FS
{
	FileSystem::CommandSupportType linuxswap::m_Create = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType linuxswap::m_Grow = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType linuxswap::m_Shrink = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType linuxswap::m_Move = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType linuxswap::m_Copy = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType linuxswap::m_GetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType linuxswap::m_SetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType linuxswap::m_GetUUID = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType linuxswap::m_UpdateUUID = FileSystem::cmdSupportNone;

	linuxswap::linuxswap(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::LinuxSwap)
	{
	}

	void linuxswap::init()
	{
		m_SetLabel = m_Shrink = m_Grow = m_Create = m_UpdateUUID = (findExternal(QStringLiteral("mkswap"))) ? cmdSupportFileSystem : cmdSupportNone;
		m_GetLabel = cmdSupportCore;
		m_Copy = cmdSupportFileSystem;
		m_Move = cmdSupportCore;
		m_GetUUID = cmdSupportCore;
	}

	bool linuxswap::supportToolFound() const
	{
		return
// 			m_GetUsed != cmdSupportNone &&
			m_GetLabel != cmdSupportNone &&
			m_SetLabel != cmdSupportNone &&
			m_Create != cmdSupportNone &&
// 			m_Check != cmdSupportNone &&
			m_UpdateUUID != cmdSupportNone &&
			m_Grow != cmdSupportNone &&
			m_Shrink != cmdSupportNone &&
			m_Copy != cmdSupportNone &&
			m_Move != cmdSupportNone &&
// 			m_Backup != cmdSupportNone &&
			m_GetUUID != cmdSupportNone;
	}

	FileSystem::SupportTool linuxswap::supportToolName() const
	{
		return SupportTool(QStringLiteral("util-linux"), QUrl(QStringLiteral("http://www.kernel.org/pub/linux/utils/util-linux-ng/")));
	}

	qint64 linuxswap::maxLabelLength() const
	{
		 return 15;
	}

	bool linuxswap::create(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("mkswap"), QStringList() << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool linuxswap::resize(Report& report, const QString& deviceNode, qint64 length) const
	{
		const QString label = readLabel(deviceNode);
		const QString uuid = readUUID(deviceNode);

		QStringList args;
		if (!label.isEmpty())
			args << QStringLiteral("-L") << label;
		if (!uuid.isEmpty())
			args << QStringLiteral("-U") << uuid;

		args << deviceNode << QString::number(length / 1024);

		ExternalCommand cmd(report, QStringLiteral("mkswap"), args);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool linuxswap::copy(Report& report, const QString& targetDeviceNode, const QString& sourceDeviceNode) const
	{
		const QString label = readLabel(sourceDeviceNode);
		const QString uuid = readUUID(sourceDeviceNode);

		QStringList args;
		if (!label.isEmpty())
			args << QStringLiteral("-L") << label;
		if (!uuid.isEmpty())
			args << QStringLiteral("-U") << uuid;

		args << targetDeviceNode;

		ExternalCommand cmd(report, QStringLiteral("mkswap"), args);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool linuxswap::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
	{
		ExternalCommand cmd(report, QStringLiteral("mkswap"), QStringList() << QStringLiteral("-L") << newLabel << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	QString linuxswap::mountTitle() const
	{
		return i18nc("@title:menu", "Activate swap");
	}

	QString linuxswap::unmountTitle() const
	{
		return i18nc("@title:menu", "Deactivate swap");
	}

	bool linuxswap::mount(const QString& deviceNode)
	{
		ExternalCommand cmd(QStringLiteral("swapon"), QStringList() << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool linuxswap::unmount(const QString& deviceNode)
	{
		ExternalCommand cmd(QStringLiteral("swapoff"), QStringList() << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool linuxswap::updateUUID(Report& report, const QString& deviceNode) const
	{
		const QString label = readLabel(deviceNode);

		QStringList args;
		if (!label.isEmpty())
			args << QStringLiteral("-L") << label;

		args << deviceNode;

		ExternalCommand cmd(report, QStringLiteral("mkswap"), args);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}
}
