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

#include "fs/xfs.h"

#include "util/externalcommand.h"
#include "util/capacity.h"
#include "util/report.h"

#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QTemporaryDir>

#include <KLocalizedString>

#include <unistd.h>

namespace FS
{
	FileSystem::CommandSupportType xfs::m_GetUsed = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType xfs::m_GetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType xfs::m_Create = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType xfs::m_Grow = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType xfs::m_Move = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType xfs::m_Check = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType xfs::m_Copy = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType xfs::m_Backup = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType xfs::m_SetLabel = FileSystem::cmdSupportNone;

	xfs::xfs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Xfs)
	{
	}

	void xfs::init()
	{
		m_GetLabel = cmdSupportCore;
		m_SetLabel = m_GetUsed = findExternal(QStringLiteral("xfs_db")) ? cmdSupportFileSystem : cmdSupportNone;
		m_Create = findExternal(QStringLiteral("mkfs.xfs")) ? cmdSupportFileSystem : cmdSupportNone;

		m_Check = findExternal(QStringLiteral("xfs_repair")) ? cmdSupportFileSystem : cmdSupportNone;
		m_Grow = (findExternal(QStringLiteral("xfs_growfs"), QStringList() << QStringLiteral("-V")) && m_Check != cmdSupportNone) ? cmdSupportFileSystem : cmdSupportNone;
		m_Copy = findExternal(QStringLiteral("xfs_copy")) ? cmdSupportFileSystem : cmdSupportNone;
		m_Move = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
		m_Backup = cmdSupportCore;
	}

	bool xfs::supportToolFound() const
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

	FileSystem::SupportTool xfs::supportToolName() const
	{
		return SupportTool(QStringLiteral("xfsprogs"), QUrl(QStringLiteral("http://oss.sgi.com/projects/xfs/")));
	}

	qint64 xfs::minCapacity() const
	{
		return 32 * Capacity::unitFactor(Capacity::Byte, Capacity::MiB);
	}

	qint64 xfs::maxCapacity() const
	{
		return Capacity::unitFactor(Capacity::Byte, Capacity::EiB);
	}

	qint64 xfs::maxLabelLength() const
	{
		 return 12;
	}

	qint64 xfs::readUsedCapacity(const QString& deviceNode) const
	{
		ExternalCommand cmd(QStringLiteral("xfs_db"), QStringList() << QStringLiteral("-c") << QStringLiteral("sb 0") << QStringLiteral("-c") << QStringLiteral("print") << deviceNode);

		if (cmd.run())
		{
			qint64 dBlocks = -1;
			QRegExp rxDBlocks(QStringLiteral("dblocks = (\\d+)"));

			if (rxDBlocks.indexIn(cmd.output()) != -1)
				dBlocks = rxDBlocks.cap(1).toLongLong();

			qint64 blockSize = -1;
			QRegExp rxBlockSize(QStringLiteral("blocksize = (\\d+)"));

			if (rxBlockSize.indexIn(cmd.output()) != -1)
				blockSize = rxBlockSize.cap(1).toLongLong();

			qint64 fdBlocks = -1;
			QRegExp rxFdBlocks(QStringLiteral("fdblocks = (\\d+)"));

			if (rxFdBlocks.indexIn(cmd.output()) != -1)
				fdBlocks = rxFdBlocks.cap(1).toLongLong();

			if (dBlocks > -1 && blockSize > -1 && fdBlocks > -1)
				return (dBlocks - fdBlocks) * blockSize;
		}

		return -1;
	}

	bool xfs::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
	{
		ExternalCommand cmd(report, QStringLiteral("xfs_db"), QStringList() << QStringLiteral("-x") << QStringLiteral("-c") << QStringLiteral("sb 0") << QStringLiteral("-c") << QStringLiteral("label ") + newLabel << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool xfs::check(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("xfs_repair"), QStringList() << QStringLiteral("-v") << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool xfs::create(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("mkfs.xfs"), QStringList() << QStringLiteral("-f") << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool xfs::copy(Report& report, const QString& targetDeviceNode, const QString& sourceDeviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("xfs_copy"), QStringList() << sourceDeviceNode << targetDeviceNode);

		// xfs_copy behaves a little strangely. It apparently kills itself at the end of main, causing QProcess
		// to report that it crashed.
		// See http://oss.sgi.com/archives/xfs/2004-11/msg00169.html
		// So we cannot rely on QProcess::exitStatus() and thus not on ExternalCommand::run() returning true.

		cmd.run(-1);
		return cmd.exitCode() == 0;
	}

	bool xfs::resize(Report& report, const QString& deviceNode, qint64) const
	{
		QTemporaryDir tempDir;
		if (!tempDir.isValid())
		{
			report.line() << xi18nc("@info/plain", "Resizing XFS file system on partition <filename>%1</filename> failed: Could not create temp dir.", deviceNode);
			return false;
		}

		bool rval = false;

		ExternalCommand mountCmd(report, QStringLiteral("mount"), QStringList() << QStringLiteral("-v") << QStringLiteral("-t") << QStringLiteral("xfs") << deviceNode << tempDir.path());

		if (mountCmd.run(-1))
		{
			ExternalCommand resizeCmd(report, QStringLiteral("xfs_growfs"), QStringList() << tempDir.path());

			if (resizeCmd.run(-1))
				rval = true;
			else
				report.line() << xi18nc("@info/plain", "Resizing XFS file system on partition <filename>%1</filename> failed: xfs_growfs failed.", deviceNode);

			ExternalCommand unmountCmd(report, QStringLiteral("umount"), QStringList() << tempDir.path());

			if (!unmountCmd.run(-1))
				report.line() << xi18nc("@info/plain", "Warning: Resizing XFS file system on partition <filename>%1</filename>: Unmount failed.", deviceNode);
		}
		else
			report.line() << xi18nc("@info/plain", "Resizing XFS file system on partition <filename>%1</filename> failed: Initial mount failed.", deviceNode);

		return rval;
	}
}
