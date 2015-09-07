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

#include "fs/jfs.h"

#include "util/externalcommand.h"
#include "util/report.h"
#include "util/capacity.h"

#include <QStringList>
#include <QRegExp>
#include <QTemporaryDir>

#include <KLocalizedString>

#include <unistd.h>

namespace FS
{
	FileSystem::CommandSupportType jfs::m_GetUsed = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType jfs::m_GetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType jfs::m_Create = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType jfs::m_Grow = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType jfs::m_Move = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType jfs::m_Check = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType jfs::m_Copy = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType jfs::m_Backup = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType jfs::m_SetLabel = FileSystem::cmdSupportNone;

	jfs::jfs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Jfs)
	{
	}

	void jfs::init()
	{
		m_GetUsed = findExternal(QStringLiteral("jfs_debugfs")) ? cmdSupportFileSystem : cmdSupportNone;
		m_GetLabel = cmdSupportCore;
		m_SetLabel = findExternal(QStringLiteral("jfs_tune"), QStringList() << QStringLiteral("-V")) ? cmdSupportFileSystem : cmdSupportNone;
		m_Create = findExternal(QStringLiteral("mkfs.jfs"), QStringList() << QStringLiteral("-V")) ? cmdSupportFileSystem : cmdSupportNone;
		m_Grow = m_Check = findExternal(QStringLiteral("fsck.jfs"), QStringList() << QStringLiteral("-V")) ? cmdSupportFileSystem : cmdSupportNone;
		m_Copy = m_Move = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
		m_Backup = cmdSupportCore;
	}

	bool jfs::supportToolFound() const
	{
		return
			m_GetUsed != cmdSupportNone &&
			m_GetLabel != cmdSupportNone &&
			m_SetLabel != cmdSupportNone &&
			m_Create != cmdSupportNone &&
			m_Check != cmdSupportNone &&
// 			m_UpdateUUID != cmdSupportNone &&
			m_Grow != cmdSupportNone &&
// 			m_Shrink != cmdSupportNone &&
			m_Copy != cmdSupportNone &&
			m_Move != cmdSupportNone &&
			m_Backup != cmdSupportNone;
// 			m_GetUUID != cmdSupportNone;
	}

	FileSystem::SupportTool jfs::supportToolName() const
	{
		return SupportTool(QStringLiteral("jfsutils"), QUrl(QStringLiteral("http://jfs.sourceforge.net/")));
	}

	qint64 jfs::minCapacity() const
	{
		return 16 * Capacity::unitFactor(Capacity::Byte, Capacity::MiB);
	}

	qint64 jfs::maxCapacity() const
	{
		return 32 * Capacity::unitFactor(Capacity::Byte, Capacity::PiB);
	}

	qint64 jfs::maxLabelLength() const
	{
		 return 11;
	}

	qint64 jfs::readUsedCapacity(const QString& deviceNode) const
	{
		ExternalCommand cmd(QStringLiteral("jfs_debugfs"), QStringList() << deviceNode);

		if (cmd.start() && cmd.write("dm") == 2 && cmd.waitFor())
		{
			qint64 blockSize = -1;
			QRegExp rxBlockSize(QStringLiteral("Block Size: (\\d+)"));

			if (rxBlockSize.indexIn(cmd.output()) != -1)
				blockSize = rxBlockSize.cap(1).toLongLong();

			qint64 nBlocks = -1;
			QRegExp rxnBlocks(QStringLiteral("dn_mapsize:\\s+0x([0-9a-f]+)"));

			bool ok = false;
			if (rxnBlocks.indexIn(cmd.output()) != -1)
			{
				nBlocks = rxnBlocks.cap(1).toLongLong(&ok, 16);
				if (!ok)
					nBlocks = -1;
			}

			qint64 nFree = -1;
			QRegExp rxnFree(QStringLiteral("dn_nfree:\\s+0x([0-9a-f]+)"));

			if (rxnFree.indexIn(cmd.output()) != -1)
			{
				nFree = rxnFree.cap(1).toLongLong(&ok, 16);
				if (!ok)
					nFree = -1;
			}

			if (nBlocks > -1 && blockSize > -1 && nFree > -1)
				return (nBlocks - nFree) * blockSize;
		}

		return -1;
	}

	bool jfs::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
	{
		ExternalCommand cmd(report, QStringLiteral("jfs_tune"), QStringList() << QStringLiteral("-L") << newLabel << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool jfs::check(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("fsck.jfs"), QStringList() << QStringLiteral("-f") << deviceNode);
		return cmd.run(-1) && (cmd.exitCode() == 0 || cmd.exitCode() == 1);
	}

	bool jfs::create(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("mkfs.jfs"), QStringList() << QStringLiteral("-q") << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool jfs::resize(Report& report, const QString& deviceNode, qint64) const
	{
		QTemporaryDir tempDir;
		if (!tempDir.isValid())
		{
			report.line() << xi18nc("@info/plain", "Resizing JFS file system on partition <filename>%1</filename> failed: Could not create temp dir.", deviceNode);
			return false;
		}

		bool rval = false;

		ExternalCommand mountCmd(report, QStringLiteral("mount"), QStringList() << QStringLiteral("-v") << QStringLiteral("-t") << QStringLiteral("jfs") << deviceNode << tempDir.path());

		if (mountCmd.run(-1))
		{
			ExternalCommand resizeMountCmd(report, QStringLiteral("mount"), QStringList() << QStringLiteral("-v") << QStringLiteral("-t") << QStringLiteral("jfs") << QStringLiteral("-o") << QStringLiteral("remount,resize") << deviceNode << tempDir.path());

			if (resizeMountCmd.run(-1))
				rval = true;
			else
				report.line() << xi18nc("@info/plain", "Resizing JFS file system on partition <filename>%1</filename> failed: Remount failed.", deviceNode);

			ExternalCommand unmountCmd(report, QStringLiteral("umount"), QStringList() << tempDir.path());

			if (!unmountCmd.run(-1))
				report.line() << xi18nc("@info/plain", "Warning: Resizing JFS file system on partition <filename>%1</filename>: Unmount failed.", deviceNode);
		}
		else
			report.line() << xi18nc("@info/plain", "Resizing JFS file system on partition <filename>%1</filename> failed: Initial mount failed.", deviceNode);

		return rval;
	}
}
