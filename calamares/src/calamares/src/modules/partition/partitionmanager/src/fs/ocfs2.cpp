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

#include "fs/ocfs2.h"

#include "util/externalcommand.h"
#include "util/capacity.h"

#include <QString>
#include <QRegExp>

namespace FS
{
	FileSystem::CommandSupportType ocfs2::m_GetUsed = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType ocfs2::m_GetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType ocfs2::m_Create = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType ocfs2::m_Grow = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType ocfs2::m_Shrink = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType ocfs2::m_Move = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType ocfs2::m_Check = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType ocfs2::m_Copy = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType ocfs2::m_Backup = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType ocfs2::m_SetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType ocfs2::m_UpdateUUID = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType ocfs2::m_GetUUID = FileSystem::cmdSupportNone;

	ocfs2::ocfs2(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Ocfs2)
	{
	}

	void ocfs2::init()
	{
		m_Create = findExternal(QStringLiteral("mkfs.ocfs2"), QStringList() << QStringLiteral("-V")) ? cmdSupportFileSystem : cmdSupportNone;
		m_Check = findExternal(QStringLiteral("fsck.ocfs2"), QStringList(), 16) ? cmdSupportFileSystem : cmdSupportNone;
		m_Grow = (m_Check != cmdSupportNone && findExternal(QStringLiteral("tunefs.ocfs2"), QStringList() << QStringLiteral("-V")) && findExternal(QStringLiteral("debugfs.ocfs2"), QStringList() << QStringLiteral("-V"))) ? cmdSupportFileSystem : cmdSupportNone;
		m_Shrink = cmdSupportNone;

		// TODO: it seems there's no way to get the FS usage with ocfs2
		m_GetUsed = cmdSupportNone;

		m_SetLabel = findExternal(QStringLiteral("tunefs.ocfs2"), QStringList() << QStringLiteral("-V")) ? cmdSupportFileSystem : cmdSupportNone;
		m_UpdateUUID = findExternal(QStringLiteral("tunefs.ocfs2"), QStringList() << QStringLiteral("-V")) ? cmdSupportFileSystem : cmdSupportNone;

		m_Copy = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
		m_Move = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;

		m_GetLabel = cmdSupportCore;
		m_Backup = cmdSupportCore;
		m_GetUUID = cmdSupportCore;
	}

	bool ocfs2::supportToolFound() const
	{
		return
// 			m_GetUsed != cmdSupportNone &&
			m_GetLabel != cmdSupportNone &&
			m_SetLabel != cmdSupportNone &&
			m_Create != cmdSupportNone &&
			m_Check != cmdSupportNone &&
			m_UpdateUUID != cmdSupportNone &&
			m_Grow != cmdSupportNone &&
// 			m_Shrink != cmdSupportNone &&
			m_Copy != cmdSupportNone &&
			m_Move != cmdSupportNone &&
			m_Backup != cmdSupportNone &&
			m_GetUUID != cmdSupportNone;
	}

	FileSystem::SupportTool ocfs2::supportToolName() const
	{
		return SupportTool(QStringLiteral("ocfs2-tools"), QUrl(QStringLiteral("http://oss.oracle.com/projects/ocfs2-tools/")));
	}

	qint64 ocfs2::minCapacity() const
	{
		 return 14000 * Capacity::unitFactor(Capacity::Byte, Capacity::KiB);
	}

	qint64 ocfs2::maxCapacity() const
	{
		 return 4 * Capacity::unitFactor(Capacity::Byte, Capacity::PiB);
	}

	qint64 ocfs2::readUsedCapacity(const QString& deviceNode) const
	{
		Q_UNUSED(deviceNode);
		return -1;
	}

	bool ocfs2::check(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("fsck.ocfs2"), QStringList() << QStringLiteral("-f") << QStringLiteral("-y") << deviceNode);
		return cmd.run(-1) && (cmd.exitCode() == 0 || cmd.exitCode() == 1 || cmd.exitCode() == 2);
	}

	bool ocfs2::create(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("mkfs.ocfs2"), QStringList() << deviceNode);

		cmd.start();
		cmd.write("y\n");
		cmd.waitFor(-1);

		return cmd.exitCode() == 0;
	}

	bool ocfs2::resize(Report& report, const QString& deviceNode, qint64 length) const
	{
		ExternalCommand cmdBlockSize(QStringLiteral("debugfs.ocfs2"), QStringList() << QStringLiteral("-R") << QStringLiteral("stats") << deviceNode);

		qint32 blockSize = -1;
		if (cmdBlockSize.run())
		{
			QRegExp rxBlockSizeBits(QStringLiteral("Block Size Bits: (\\d+)"));

			if (rxBlockSizeBits.indexIn(cmdBlockSize.output()) != -1)
				blockSize = 1 << rxBlockSizeBits.cap(1).toInt();
		}

		if (blockSize == -1)
			return false;

		ExternalCommand cmd(report, QStringLiteral("tunefs.ocfs2"), QStringList() << QStringLiteral("-y") << QStringLiteral("-S") << deviceNode << QString::number(length / blockSize));
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool ocfs2::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
	{
		ExternalCommand cmd(report, QStringLiteral("tunefs.ocfs2"), QStringList() << QStringLiteral("-L") << newLabel << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool ocfs2::updateUUID(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("tunefs.ocfs2"), QStringList() << QStringLiteral("-U") << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}
}
