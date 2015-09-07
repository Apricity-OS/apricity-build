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

#include "fs/reiserfs.h"

#include "util/externalcommand.h"
#include "util/capacity.h"

#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QUuid>

namespace FS
{
	FileSystem::CommandSupportType reiserfs::m_GetUsed = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType reiserfs::m_GetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType reiserfs::m_Create = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType reiserfs::m_Grow = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType reiserfs::m_Shrink = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType reiserfs::m_Move = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType reiserfs::m_Check = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType reiserfs::m_Copy = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType reiserfs::m_Backup = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType reiserfs::m_SetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType reiserfs::m_UpdateUUID = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType reiserfs::m_GetUUID = FileSystem::cmdSupportNone;

	reiserfs::reiserfs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::ReiserFS)
	{
	}

	void reiserfs::init()
	{
		m_GetLabel = cmdSupportCore;
		m_GetUsed = findExternal(QStringLiteral("debugreiserfs"), QStringList(), 16) ? cmdSupportFileSystem : cmdSupportNone;
		m_SetLabel = findExternal(QStringLiteral("reiserfstune")) ? cmdSupportFileSystem : cmdSupportNone;
		m_Create = findExternal(QStringLiteral("mkfs.reiserfs")) ? cmdSupportFileSystem : cmdSupportNone;
		m_Check = findExternal(QStringLiteral("fsck.reiserfs")) ? cmdSupportFileSystem : cmdSupportNone;
		m_Move = m_Copy = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
		m_Grow = findExternal(QStringLiteral("resize_reiserfs"), QStringList(), 16) ? cmdSupportFileSystem : cmdSupportNone;
		m_Shrink = (m_GetUsed != cmdSupportNone && m_Grow != cmdSupportNone) ? cmdSupportFileSystem : cmdSupportNone;
		m_Backup = cmdSupportCore;
		m_UpdateUUID = findExternal(QStringLiteral("reiserfstune")) ? cmdSupportFileSystem : cmdSupportNone;
		m_GetUUID = cmdSupportCore;
	}

	bool reiserfs::supportToolFound() const
	{
		return
			m_GetUsed != cmdSupportNone &&
			m_GetLabel != cmdSupportNone &&
			m_SetLabel != cmdSupportNone &&
			m_Create != cmdSupportNone &&
			m_Check != cmdSupportNone &&
			m_UpdateUUID != cmdSupportNone &&
			m_Grow != cmdSupportNone &&
			m_Shrink != cmdSupportNone &&
			m_Copy != cmdSupportNone &&
			m_Move != cmdSupportNone &&
			m_Backup != cmdSupportNone &&
			m_GetUUID != cmdSupportNone;
	}

	FileSystem::SupportTool reiserfs::supportToolName() const
	{
		return SupportTool(QStringLiteral("reiserfsprogs"), QUrl(QStringLiteral("http://www.kernel.org/pub/linux/utils/fs/reiserfs/")));
	}

	qint64 reiserfs::minCapacity() const
	{
		return 32 * Capacity::unitFactor(Capacity::Byte, Capacity::MiB);
	}

	qint64 reiserfs::maxCapacity() const
	{
		return 16 * Capacity::unitFactor(Capacity::Byte, Capacity::TiB);
	}

	qint64 reiserfs::maxLabelLength() const
	{
		 return 16;
	}

	qint64 reiserfs::readUsedCapacity(const QString& deviceNode) const
	{
		ExternalCommand cmd(QStringLiteral("debugreiserfs"), QStringList() << deviceNode);

		if (cmd.run())
		{
			qint64 blockCount = -1;
			QRegExp rxBlockCount(QStringLiteral("Count of blocks[^:]+: (\\d+)"));

			if (rxBlockCount.indexIn(cmd.output()) != -1)
				blockCount = rxBlockCount.cap(1).toLongLong();

			qint64 blockSize = -1;
			QRegExp rxBlockSize(QStringLiteral("Blocksize: (\\d+)"));

			if (rxBlockSize.indexIn(cmd.output()) != -1)
				blockSize = rxBlockSize.cap(1).toLongLong();

			qint64 freeBlocks = -1;
			QRegExp rxFreeBlocks(QStringLiteral("Free blocks[^:]+: (\\d+)"));

			if (rxFreeBlocks.indexIn(cmd.output()) != -1)
				freeBlocks = rxFreeBlocks.cap(1).toLongLong();

			if (blockCount > -1 && blockSize > -1 && freeBlocks > -1)
				return (blockCount - freeBlocks) * blockSize;
		}

		return -1;
	}

	bool reiserfs::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
	{
		ExternalCommand cmd(report, QStringLiteral("reiserfstune"), QStringList() << QStringLiteral("-l") << newLabel << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool reiserfs::check(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("fsck.reiserfs"), QStringList() << QStringLiteral("--fix-fixable") << QStringLiteral("-q") << QStringLiteral("-y") << deviceNode);
		return cmd.run(-1) && (cmd.exitCode() == 0 || cmd.exitCode() == 1 || cmd.exitCode() == 256);
	}

	bool reiserfs::create(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("mkfs.reiserfs"), QStringList() << QStringLiteral("-f") << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool reiserfs::resize(Report& report, const QString& deviceNode, qint64 length) const
	{
		ExternalCommand cmd(report, QStringLiteral("resize_reiserfs"), QStringList() << deviceNode << QStringLiteral("-q") << QStringLiteral("-s") << QString::number(length));

		bool rval = cmd.start(-1);

		if (!rval)
			return false;

		if (cmd.write("y\n", 2) != 2)
			return false;

		return cmd.waitFor(-1) && (cmd.exitCode() == 0 || cmd.exitCode() == 256);
	}

	bool reiserfs::updateUUID(Report& report, const QString& deviceNode) const
	{
		const QString uuid = QUuid::createUuid().toString().remove(QRegExp(QStringLiteral("\\{|\\}")));
		ExternalCommand cmd(report, QStringLiteral("reiserfstune"), QStringList() << QStringLiteral("-u") << uuid << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}
}
