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

#include "fs/ntfs.h"

#include "util/externalcommand.h"
#include "util/capacity.h"
#include "util/report.h"
#include "util/globallog.h"

#include <KLocalizedString>

#include <QString>
#include <QStringList>
#include <QFile>
#include <QUuid>

#include <ctime>
#include <algorithm>

namespace FS
{
	FileSystem::CommandSupportType ntfs::m_GetUsed = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType ntfs::m_GetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType ntfs::m_Create = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType ntfs::m_Grow = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType ntfs::m_Shrink = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType ntfs::m_Move = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType ntfs::m_Check = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType ntfs::m_Copy = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType ntfs::m_Backup = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType ntfs::m_SetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType ntfs::m_UpdateUUID = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType ntfs::m_GetUUID = FileSystem::cmdSupportNone;

	ntfs::ntfs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Ntfs)
	{
	}

	void ntfs::init()
	{
		m_Shrink = m_Grow = m_Check = m_GetUsed = findExternal(QStringLiteral("ntfsresize")) ? cmdSupportFileSystem : cmdSupportNone;
		m_GetLabel = cmdSupportCore;
		m_SetLabel = findExternal(QStringLiteral("ntfslabel")) ? cmdSupportFileSystem : cmdSupportNone;
		m_Create = findExternal(QStringLiteral("mkfs.ntfs")) ? cmdSupportFileSystem : cmdSupportNone;
		m_Copy = findExternal(QStringLiteral("ntfsclone")) ? cmdSupportFileSystem : cmdSupportNone;
		m_Backup = cmdSupportCore;
		m_UpdateUUID = findExternal(QStringLiteral("dd")) ? cmdSupportFileSystem : cmdSupportNone;
		m_Move = (m_Check != cmdSupportNone) ? cmdSupportCore : cmdSupportNone;
		m_GetUUID = cmdSupportCore;
	}

	bool ntfs::supportToolFound() const
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

	FileSystem::SupportTool ntfs::supportToolName() const
	{
		return SupportTool(QStringLiteral("ntfsprogs"), QUrl(QStringLiteral("http://www.linux-ntfs.org/doku.php?id=ntfsprogs")));
	}

	qint64 ntfs::minCapacity() const
	{
		return 2 * Capacity::unitFactor(Capacity::Byte, Capacity::MiB);
	}

	qint64 ntfs::maxCapacity() const
	{
		return 256 * Capacity::unitFactor(Capacity::Byte, Capacity::TiB);
	}

	qint64 ntfs::maxLabelLength() const
	{
		 return 128;
	}

	qint64 ntfs::readUsedCapacity(const QString& deviceNode) const
	{
		ExternalCommand cmd(QStringLiteral("ntfsresize"), QStringList() << QStringLiteral("--info") << QStringLiteral("--force") << QStringLiteral("--no-progress-bar") << deviceNode);

		if (cmd.run())
		{
			qint64 usedBytes = -1;
			QRegExp rxUsedBytes(QStringLiteral("resize at (\\d+) bytes"));

			if (rxUsedBytes.indexIn(cmd.output()) != -1)
				usedBytes = rxUsedBytes.cap(1).toLongLong();

			if (usedBytes > -1)
				return usedBytes;
		}

		return -1;
	}

	bool ntfs::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
	{
		ExternalCommand writeCmd(report, QStringLiteral("ntfslabel"), QStringList() << QStringLiteral("--force") << deviceNode << newLabel.simplified());
		writeCmd.setProcessChannelMode(QProcess::SeparateChannels);

		if (!writeCmd.run(-1))
			return false;

		ExternalCommand testCmd(QStringLiteral("ntfslabel"), QStringList() << QStringLiteral("--force") << deviceNode);
		testCmd.setProcessChannelMode(QProcess::SeparateChannels);

		if (!testCmd.run(-1))
			return false;

		return testCmd.output().simplified() == newLabel.simplified();
	}

	bool ntfs::check(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("ntfsresize"), QStringList() << QStringLiteral("-P") << QStringLiteral("-i") << QStringLiteral("-f") << QStringLiteral("-v") << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool ntfs::create(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("mkfs.ntfs"), QStringList() << QStringLiteral("-f") << QStringLiteral("-vv") << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool ntfs::copy(Report& report, const QString& targetDeviceNode, const QString& sourceDeviceNode) const
	{
 		ExternalCommand cmd(report, QStringLiteral("ntfsclone"), QStringList() << QStringLiteral("-f") << QStringLiteral("--overwrite") << targetDeviceNode << sourceDeviceNode);

 		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool ntfs::resize(Report& report, const QString& deviceNode, qint64 length) const
	{
		QStringList args;
		args << QStringLiteral("-P") << QStringLiteral("-f") << deviceNode << QStringLiteral("-s") << QString::number(length);

		QStringList dryRunArgs = args;
		dryRunArgs << QStringLiteral("-n");
		ExternalCommand cmdDryRun(QStringLiteral("ntfsresize"), dryRunArgs);

		if (cmdDryRun.run(-1) && cmdDryRun.exitCode() == 0)
		{
			ExternalCommand cmd(report, QStringLiteral("ntfsresize"), args);
			return cmd.run(-1) && cmd.exitCode() == 0;
		}

		return false;
	}

	bool ntfs::updateUUID(Report& report, const QString& deviceNode) const
	{
		QUuid uuid = QUuid::createUuid();

		ExternalCommand cmd(report, QStringLiteral("dd"), QStringList() << QStringLiteral("of=") + deviceNode << QStringLiteral("bs=1") << QStringLiteral("count=8") << QStringLiteral("seek=72"));

		if (!cmd.start())
			return false;

		if (cmd.write(reinterpret_cast<char*>(&uuid.data4[0]), 8) != 8)
			return false;

		return cmd.waitFor(-1);
	}

	bool ntfs::updateBootSector(Report& report, const QString& deviceNode) const
	{
		report.line() << xi18nc("@info/plain", "Updating boot sector for NTFS file system on partition <filename>%1</filename>.", deviceNode);

		quint32 n = firstSector();
		char* s = reinterpret_cast<char*>(&n);

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
		std::swap(s[0], s[3]);
		std::swap(s[1], s[2]);
#endif

		QFile device(deviceNode);
		if (!device.open(QFile::ReadWrite | QFile::Unbuffered))
		{
			Log() << xi18nc("@info/plain", "Could not open partition <filename>%1</filename> for writing when trying to update the NTFS boot sector.", deviceNode);
			return false;
		}

		if (!device.seek(0x1c))
		{
			Log() << xi18nc("@info/plain", "Could not seek to position 0x1c on partition <filename>%1</filename> when trying to update the NTFS boot sector.", deviceNode);
			return false;
		}

		if (device.write(s, 4) != 4)
		{
			Log() << xi18nc("@info/plain", "Could not write new start sector to partition <filename>%1</filename> when trying to update the NTFS boot sector.", deviceNode);
			return false;
		}

		Log() << xi18nc("@info/plain", "Updated NTFS boot sector for partition <filename>%1</filename> successfully.", deviceNode);

		return true;
	}
}
