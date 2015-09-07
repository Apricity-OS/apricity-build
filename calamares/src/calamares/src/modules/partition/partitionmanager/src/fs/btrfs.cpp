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

#include "fs/btrfs.h"

#include "util/externalcommand.h"
#include "util/capacity.h"
#include "util/report.h"

#include <QString>
#include <QRegExp>
#include <QTemporaryDir>

#include <KLocalizedString>

namespace FS
{
	FileSystem::CommandSupportType btrfs::m_GetUsed = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType btrfs::m_GetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType btrfs::m_Create = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType btrfs::m_Grow = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType btrfs::m_Shrink = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType btrfs::m_Move = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType btrfs::m_Check = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType btrfs::m_Copy = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType btrfs::m_Backup = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType btrfs::m_SetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType btrfs::m_UpdateUUID = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType btrfs::m_GetUUID = FileSystem::cmdSupportNone;

	btrfs::btrfs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Btrfs)
	{
	}

	void btrfs::init()
	{
		m_Create = findExternal(QStringLiteral("mkfs.btrfs")) ? cmdSupportFileSystem : cmdSupportNone;
		m_Check = findExternal(QStringLiteral("btrfsck"), QStringList(), 1) ? cmdSupportFileSystem : cmdSupportNone;
		m_Grow = (m_Check != cmdSupportNone && findExternal(QStringLiteral("btrfs"))) ? cmdSupportFileSystem : cmdSupportNone;
		m_GetUsed = findExternal(QStringLiteral("btrfs-debug-tree")) ? cmdSupportFileSystem : cmdSupportNone;
		m_Shrink = (m_Grow != cmdSupportNone && m_GetUsed != cmdSupportNone) ? cmdSupportFileSystem : cmdSupportNone;

		m_SetLabel = findExternal(QStringLiteral("btrfs")) ? cmdSupportFileSystem : cmdSupportNone;
		m_UpdateUUID = cmdSupportNone;

		m_Copy = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
		m_Move = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;

		m_GetLabel = cmdSupportCore;
		m_Backup = cmdSupportCore;
		m_GetUUID = cmdSupportCore;
	}

	bool btrfs::supportToolFound() const
	{
		return
			m_GetUsed != cmdSupportNone &&
			m_GetLabel != cmdSupportNone &&
			m_SetLabel != cmdSupportNone &&
			m_Create != cmdSupportNone &&
			m_Check != cmdSupportNone &&
// 			m_UpdateUUID != cmdSupportNone &&
			m_Grow != cmdSupportNone &&
			m_Shrink != cmdSupportNone &&
			m_Copy != cmdSupportNone &&
			m_Move != cmdSupportNone &&
			m_Backup != cmdSupportNone &&
			m_GetUUID != cmdSupportNone;
	}

	FileSystem::SupportTool btrfs::supportToolName() const
	{
		return SupportTool(QStringLiteral("btrfs-tools"), QUrl(QStringLiteral("http://btrfs.wiki.kernel.org/")));
	}

	qint64 btrfs::minCapacity() const
	{
		 return 256 * Capacity::unitFactor(Capacity::Byte, Capacity::MiB);
	}

	qint64 btrfs::maxCapacity() const
	{
		 return Capacity::unitFactor(Capacity::Byte, Capacity::EiB);
	}

	qint64 btrfs::maxLabelLength() const
	{
		 return 255;
	}

	qint64 btrfs::readUsedCapacity(const QString& deviceNode) const
	{
		ExternalCommand cmd(QStringLiteral("btrfs-debug-tree"), QStringList() << deviceNode);

		if (cmd.run())
		{
			QRegExp rxBytesUsed(QStringLiteral(" bytes used (\\d+)"));

			if (rxBytesUsed.indexIn(cmd.output()) != -1)
				return rxBytesUsed.cap(1).toLongLong();
		}

		return -1;
	}

	bool btrfs::check(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("btrfsck"), QStringList() << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool btrfs::create(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("mkfs.btrfs"), QStringList() << QStringLiteral("-f") << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool btrfs::resize(Report& report, const QString& deviceNode, qint64 length) const
	{
		QTemporaryDir tempDir;
		if (!tempDir.isValid())
		{
			report.line() << xi18nc("@info/plain", "Resizing Btrfs file system on partition <filename>%1</filename> failed: Could not create temp dir.", deviceNode);
			return false;
		}

		bool rval = false;

		ExternalCommand mountCmd(report, QStringLiteral("mount"), QStringList() << QStringLiteral("-v") << QStringLiteral("-t") << QStringLiteral("btrfs") << deviceNode << tempDir.path());

		if (mountCmd.run(-1) && mountCmd.exitCode() == 0)
		{
			ExternalCommand resizeCmd(report, QStringLiteral("btrfs"), QStringList() << QStringLiteral("filesystem") << QStringLiteral("resize") << QString::number(length) << tempDir.path());

			if (resizeCmd.run(-1) && resizeCmd.exitCode() == 0)
				rval = true;
			else
				report.line() << xi18nc("@info/plain", "Resizing Btrfs file system on partition <filename>%1</filename> failed: btrfs file system resize failed.", deviceNode);

			ExternalCommand unmountCmd(report, QStringLiteral("umount"), QStringList() << tempDir.path());

			if (!unmountCmd.run(-1) && unmountCmd.exitCode() == 0 )
				report.line() << xi18nc("@info/plain", "Warning: Resizing Btrfs file system on partition <filename>%1</filename>: Unmount failed.", deviceNode);
		}
		else
			report.line() << xi18nc("@info/plain", "Resizing Btrfs file system on partition <filename>%1</filename> failed: Initial mount failed.", deviceNode);

		return rval;
	}

	bool btrfs::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
	{
		ExternalCommand cmd(report, QStringLiteral("btrfs"), QStringList() << QStringLiteral("filesystem") << QStringLiteral("label") << deviceNode << newLabel);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool btrfs::updateUUID(Report& report, const QString& deviceNode) const
	{
		Q_UNUSED(report);
		Q_UNUSED(deviceNode);

		return false;
	}
}
