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

#include "fs/fat32.h"

#include "util/externalcommand.h"
#include "util/capacity.h"

#include <QStringList>

#include <ctime>

namespace FS
{
	fat32::fat32(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		fat16(firstsector, lastsector, sectorsused, label, FileSystem::Fat32)
	{
	}

	qint64 fat32::minCapacity() const
	{
		return 32 * Capacity::unitFactor(Capacity::Byte, Capacity::MiB);
	}

	qint64 fat32::maxCapacity() const
	{
		return 16 * Capacity::unitFactor(Capacity::Byte, Capacity::TiB);
	}

	bool fat32::create(Report& report, const QString& deviceNode) const
	{
		ExternalCommand cmd(report, QStringLiteral("mkfs.msdos"), QStringList() << QStringLiteral("-F32") << QStringLiteral("-I") << QStringLiteral("-v") << deviceNode);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	bool fat32::updateUUID(Report& report, const QString& deviceNode) const
	{
		qint32 t = time(NULL);

		char uuid[4];
		for (quint32 i = 0; i < sizeof(uuid); i++, t >>= 8)
			uuid[i] = t & 0xff;

		ExternalCommand cmd(report, QStringLiteral("dd"), QStringList() << QStringLiteral("of=") + deviceNode << QStringLiteral("bs=1") << QStringLiteral("count=4") << QStringLiteral("seek=67"));

		if (!cmd.start())
			return false;

		if (cmd.write(uuid, sizeof(uuid)) != sizeof(uuid))
			return false;

		return cmd.waitFor(-1);
	}
}
