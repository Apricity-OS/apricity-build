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

#include "fs/reiser4.h"

#include "util/capacity.h"
#include "util/externalcommand.h"

#include <QStringList>
#include <QRegExp>

namespace FS
{
	FileSystem::CommandSupportType reiser4::m_GetUsed = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType reiser4::m_GetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType reiser4::m_Create = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType reiser4::m_Move = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType reiser4::m_Check = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType reiser4::m_Copy = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType reiser4::m_Backup = FileSystem::cmdSupportNone;

	reiser4::reiser4(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Reiser4)
	{
	}

	void reiser4::init()
	{
		m_GetLabel = cmdSupportCore;
		m_GetUsed = findExternal(QStringLiteral("debugfs.reiser4"), QStringList(), 16) ? cmdSupportFileSystem : cmdSupportNone;
		m_Create = findExternal(QStringLiteral("mkfs.reiser4"), QStringList(), 16) ? cmdSupportFileSystem : cmdSupportNone;
		m_Check = findExternal(QStringLiteral("fsck.reiser4"), QStringList(), 16) ? cmdSupportFileSystem : cmdSupportNone;
		m_Move = m_Copy = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
		m_Backup = cmdSupportCore;
	}

	bool reiser4::supportToolFound() const
	{
		return
			m_GetUsed != cmdSupportNone &&
			m_GetLabel != cmdSupportNone &&
// 			m_SetLabel != cmdSupportNone &&
			m_Create != cmdSupportNone &&
			m_Check != cmdSupportNone &&
// 			m_UpdateUUID != cmdSupportNone &&
// 			m_Grow != cmdSupportNone &&
// 			m_Shrink != cmdSupportNone &&
			m_Copy != cmdSupportNone &&
			m_Move != cmdSupportNone &&
			m_Backup != cmdSupportNone;
// 			m_GetUUID != cmdSupportNone;
	}

	FileSystem::SupportTool reiser4::supportToolName() const
	{
		return SupportTool(QStringLiteral("reiser4progs"), QUrl(QStringLiteral("http://www.kernel.org/pub/linux/utils/fs/reiser4/reiser4progs/")));
	}

	qint64 reiser4::maxCapacity() const
	{
		// looks like it's actually unknown. see
		// http://en.wikipedia.org/wiki/Comparison_of_file_systems
		return Capacity::unitFactor(Capacity::Byte, Capacity::EiB);
	}

	qint64 reiser4::maxLabelLength() const
	{
		 return 16;
	}

	qint64 reiser4::readUsedCapacity(const QString& deviceNode) const
	{
		ExternalCommand cmd(QStringLiteral("debugfs.reiser4"), QStringList() << deviceNode);

		if (cmd.run())
		{
			qint64 blocks = -1;
			QRegExp rxBlocks(QStringLiteral("blocks:\\s+(\\d+)"));

			if (rxBlocks.indexIn(cmd.output()) != -1)
				blocks = rxBlocks.cap(1).toLongLong();

			qint64 blockSize = -1;
			QRegExp rxBlockSize(QStringLiteral("blksize:\\s+(\\d+)"));

			if (rxBlockSize.indexIn(cmd.output()) != -1)
				blockSize = rxBlockSize.cap(1).toLongLong();

			qint64 freeBlocks = -1;
			QRegExp rxFreeBlocks(QStringLiteral("free blocks:\\s+(\\d+)"));

			if (rxFreeBlocks.indexIn(cmd.output()) != -1)
				freeBlocks = rxFreeBlocks.cap(1).toLongLong();

			if (blocks > - 1 && blockSize > -1 && freeBlocks > -1)
				return (blocks - freeBlocks) * blockSize;
		}

		return -1;
	}

	bool reiser4::check(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("fsck.reiser4"), QStringList() << QStringLiteral("--fix") << QStringLiteral("-y") << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool reiser4::create(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("mkfs.reiser4"), QStringList() << QStringLiteral("--yes") << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}
}
