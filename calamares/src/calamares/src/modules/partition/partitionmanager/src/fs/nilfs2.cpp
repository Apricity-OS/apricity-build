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

#include "fs/nilfs2.h"

#include "util/externalcommand.h"
#include "util/capacity.h"
#include "util/report.h"

#include <cmath>

#include <QString>
#include <QTemporaryDir>
#include <QUuid>

#include <KLocalizedString>

namespace FS
{
	FileSystem::CommandSupportType nilfs2::m_GetUsed = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType nilfs2::m_GetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType nilfs2::m_Create = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType nilfs2::m_Grow = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType nilfs2::m_Shrink = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType nilfs2::m_Move = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType nilfs2::m_Check = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType nilfs2::m_Copy = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType nilfs2::m_Backup = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType nilfs2::m_SetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType nilfs2::m_UpdateUUID = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType nilfs2::m_GetUUID = FileSystem::cmdSupportNone;

	nilfs2::nilfs2(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Nilfs2)
	{
	}

	void nilfs2::init()
	{
		m_Create = findExternal(QStringLiteral("mkfs.nilfs2")) ? cmdSupportFileSystem : cmdSupportNone;
		m_Check = findExternal(QStringLiteral("fsck.nilfs2")) ? cmdSupportFileSystem : cmdSupportNone;

		m_GetLabel = cmdSupportCore;
		m_SetLabel = findExternal(QStringLiteral("nilfs-tune")) ? cmdSupportFileSystem : cmdSupportNone;
		m_UpdateUUID = findExternal(QStringLiteral("nilfs-tune")) ? cmdSupportFileSystem : cmdSupportNone;

		m_Grow = (m_Check != cmdSupportNone && findExternal(QStringLiteral("nilfs-resize"))) ? cmdSupportFileSystem : cmdSupportNone;
		m_GetUsed = findExternal(QStringLiteral("nilfs-tune")) ? cmdSupportFileSystem : cmdSupportNone;
		m_Shrink = (m_Grow != cmdSupportNone && m_GetUsed != cmdSupportNone) ? cmdSupportFileSystem : cmdSupportNone;

		m_Copy = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
		m_Move = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;

		m_GetLabel = cmdSupportCore;
		m_Backup = cmdSupportCore;
		m_GetUUID = cmdSupportCore;
	}

	bool nilfs2::supportToolFound() const
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

	FileSystem::SupportTool nilfs2::supportToolName() const
	{
		return SupportTool(QStringLiteral("nilfs2-utils"), QUrl(QStringLiteral("http://code.google.com/p/nilfs2/")));
	}

	qint64 nilfs2::minCapacity() const
	{
		 return 128 * Capacity::unitFactor(Capacity::Byte, Capacity::MiB);
	}

	qint64 nilfs2::maxCapacity() const
	{
		 return Capacity::unitFactor(Capacity::Byte, Capacity::EiB);
	}

	qint64 nilfs2::maxLabelLength() const
	{
		 return 80;
	}

	bool nilfs2::check(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("fsck.nilfs2"), QStringList() << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool nilfs2::create(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("mkfs.nilfs2"), QStringList() << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	qint64 nilfs2::readUsedCapacity(const QString& deviceNode) const
	{
		ExternalCommand cmd(QStringLiteral("nilfs-tune"), QStringList() << QStringLiteral("-l") << deviceNode);

		if (cmd.run())
		{
			QRegExp rxBlockSize(QStringLiteral("(?:Block size:\\s+)(\\d+)"));
			QRegExp rxDeviceSize(QStringLiteral("(?:Device size:\\s+)(\\d+)"));
			QRegExp rxFreeBlocks(QStringLiteral("(?:Free blocks count:\\s+)(\\d+)"));
			if (rxBlockSize.indexIn(cmd.output()) != -1 && rxDeviceSize.indexIn(cmd.output()) != -1 && rxFreeBlocks.indexIn(cmd.output()) != -1)
				return rxDeviceSize.cap(1).toLongLong() - rxBlockSize.cap(1).toLongLong() * rxFreeBlocks.cap(1).toLongLong();
		}

		return -1;
	}

	bool nilfs2::resize(Report& report, const QString& deviceNode, qint64 length) const
	{
		QTemporaryDir tempDir;
		if (!tempDir.isValid())
		{
			report.line() << xi18nc("@info/plain", "Resizing NILFS2 file system on partition <filename>%1</filename> failed: Could not create temp dir.", deviceNode);
			return false;
		}

		bool rval = false;

		ExternalCommand mountCmd(report, QStringLiteral("mount"), QStringList() << QStringLiteral("-v") << QStringLiteral("-t") << QStringLiteral("nilfs2") << deviceNode << tempDir.path());

		if (mountCmd.run(-1) && mountCmd.exitCode() == 0)
		{
			ExternalCommand resizeCmd(report, QStringLiteral("nilfs-resize"), QStringList() << QStringLiteral("-v") << QStringLiteral("-y") << deviceNode << QString::number(length));

			if (resizeCmd.run(-1) && resizeCmd.exitCode() == 0)
				rval = true;
			else
				report.line() << xi18nc("@info/plain", "Resizing NILFS2 file system on partition <filename>%1</filename> failed: NILFS2 file system resize failed.", deviceNode);

			ExternalCommand unmountCmd(report, QStringLiteral("umount"), QStringList() << tempDir.path());

			if (!unmountCmd.run(-1) && unmountCmd.exitCode() == 0 )
				report.line() << xi18nc("@info/plain", "Warning: Resizing NILFS2 file system on partition <filename>%1</filename>: Unmount failed.", deviceNode);
		}
		else
			report.line() << xi18nc("@info/plain", "Resizing NILFS2 file system on partition <filename>%1</filename> failed: Initial mount failed.", deviceNode);

		return rval;
	}

	bool nilfs2::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
	{
		ExternalCommand cmd(report, QStringLiteral("nilfs-tune"), QStringList() << QStringLiteral("-l") << newLabel << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool nilfs2::updateUUID(Report& report, const QString& deviceNode) const
	{
		QUuid uuid = QUuid::createUuid();
		ExternalCommand cmd(report, QStringLiteral("nilfs-tune"), QStringList() << QStringLiteral("-U") << uuid.toString() << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}
}
