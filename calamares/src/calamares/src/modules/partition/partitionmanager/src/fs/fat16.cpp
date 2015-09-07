/***************************************************************************
 *   Copyright (C) 2008,2009,2011 by Volker Lanz <vl@fidra.de>             *
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

#include "fs/fat16.h"

#include "util/externalcommand.h"
#include "util/capacity.h"
#include "util/report.h"

#include "fatlabel/fatlabel.h"

#include <KLocalizedString>

#include <QString>
#include <QStringList>
#include <QRegExp>

#include <ctime>

namespace FS
{
	FileSystem::CommandSupportType fat16::m_GetUsed = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType fat16::m_GetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType fat16::m_SetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType fat16::m_Create = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType fat16::m_Grow = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType fat16::m_Shrink = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType fat16::m_Move = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType fat16::m_Check = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType fat16::m_Copy = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType fat16::m_Backup = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType fat16::m_UpdateUUID = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType fat16::m_GetUUID = FileSystem::cmdSupportNone;

	fat16::fat16(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, FileSystem::Type t) :
		FileSystem(firstsector, lastsector, sectorsused, label, t)
	{
	}

	void fat16::init()
	{
		m_Create = findExternal(QStringLiteral("mkfs.msdos")) ? cmdSupportFileSystem : cmdSupportNone;
		m_GetUsed = m_Check = findExternal(QStringLiteral("fsck.msdos"), QStringList(), 2) ? cmdSupportFileSystem : cmdSupportNone;
		m_GetLabel = cmdSupportCore;
		m_SetLabel = cmdSupportFileSystem;
		m_Move = cmdSupportCore;
		m_Copy = cmdSupportCore;
		m_Backup = cmdSupportCore;
		m_UpdateUUID = findExternal(QStringLiteral("dd")) ? cmdSupportFileSystem : cmdSupportNone;
		m_GetUUID = cmdSupportCore;
	}

	bool fat16::supportToolFound() const
	{
		return
			m_GetUsed != cmdSupportNone &&
			m_GetLabel != cmdSupportNone &&
			m_SetLabel != cmdSupportNone &&
			m_Create != cmdSupportNone &&
			m_Check != cmdSupportNone &&
			m_UpdateUUID != cmdSupportNone &&
//			m_Grow != cmdSupportNone &&
//			m_Shrink != cmdSupportNone &&
			m_Copy != cmdSupportNone &&
			m_Move != cmdSupportNone &&
			m_Backup != cmdSupportNone &&
			m_GetUUID != cmdSupportNone;
	}

	FileSystem::SupportTool fat16::supportToolName() const
	{
		// also, dd for updating the UUID, but let's assume it's there ;-)
		return SupportTool(QStringLiteral("dosfstools"), QUrl(QStringLiteral("http://www.daniel-baumann.ch/software/dosfstools/")));
	}


	qint64 fat16::minCapacity() const
	{
		return 16 * Capacity::unitFactor(Capacity::Byte, Capacity::MiB);
	}

	qint64 fat16::maxCapacity() const
	{
		return 4 * Capacity::unitFactor(Capacity::Byte, Capacity::GiB);
	}

	qint64 fat16::maxLabelLength() const
	{
		 return 11;
	}

	qint64 fat16::readUsedCapacity(const QString& deviceNode) const
	{
		ExternalCommand cmd(QStringLiteral("fsck.msdos"), QStringList() << QStringLiteral("-n") << QStringLiteral("-v") << deviceNode);

		if (cmd.run())
		{
			qint64 usedClusters = -1;
			QRegExp rxClusters(QStringLiteral("files, (\\d+)/\\d+ "));

			if (rxClusters.indexIn(cmd.output()) != -1)
				usedClusters = rxClusters.cap(1).toLongLong();

			qint64 clusterSize = -1;

			QRegExp rxClusterSize(QStringLiteral("(\\d+) bytes per cluster"));

			if (rxClusterSize.indexIn(cmd.output()) != -1)
				clusterSize = rxClusterSize.cap(1).toLongLong();

			if (usedClusters > -1 && clusterSize > -1)
				return usedClusters * clusterSize;
		}

		return -1;
	}

	bool fat16::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
	{
		report.line() << xi18nc("@info/plain", "Setting label for partition <filename>%1</filename> to %2", deviceNode, newLabel);

		return fatlabel_set_label(deviceNode.toLocal8Bit().constData(), newLabel.toLocal8Bit().constData()) == 0;
	}

	bool fat16::check(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("fsck.msdos"), QStringList() << QStringLiteral("-a") << QStringLiteral("-w") << QStringLiteral("-v") << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool fat16::create(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("mkfs.msdos"), QStringList() << QStringLiteral("-F16") << QStringLiteral("-I") << QStringLiteral("-v") << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool fat16::updateUUID(Report& report, const QString& deviceNode) const
	{
		qint32 t = time(NULL);

		char uuid[4];
		for (quint32 i = 0; i < sizeof(uuid); i++, t >>= 8)
			uuid[i] = t & 0xff;

		ExternalCommand cmd(report, QStringLiteral("dd"), QStringList() << QStringLiteral("of=") + deviceNode << QStringLiteral("bs=1") << QStringLiteral("count=4") << QStringLiteral("seek=39"));

		if (!cmd.start())
			return false;

		if (cmd.write(uuid, sizeof(uuid)) != sizeof(uuid))
			return false;

		return cmd.waitFor(-1);
	}
}
