/***************************************************************************
 *   Copyright (C) 2010, 2012 by Volker Lanz <vl@fidra.de                  *
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

#include "plugins/dummy/dummypartitiontable.h"
#include "plugins/dummy/dummypartition.h"
#include "plugins/dummy/dummybackend.h"

#include "core/partition.h"
#include "core/device.h"

#include "fs/filesystem.h"

#include "util/report.h"

#include <unistd.h>

DummyPartitionTable::DummyPartitionTable() :
	CoreBackendPartitionTable()
{
}

DummyPartitionTable::~DummyPartitionTable()
{
}

bool DummyPartitionTable::open()
{
	return true;
}


bool DummyPartitionTable::commit(quint32 timeout)
{
	Q_UNUSED(timeout);

	return true;
}

CoreBackendPartition* DummyPartitionTable::getExtendedPartition()
{
	return new DummyPartition();
}

CoreBackendPartition* DummyPartitionTable::getPartitionBySector(qint64 sector)
{
	Q_UNUSED(sector);

	return new DummyPartition();
}

QString DummyPartitionTable::createPartition(Report& report, const Partition& partition)
{
	Q_UNUSED(report);
	Q_UNUSED(partition);

	return QStringLiteral("dummy");
}

bool DummyPartitionTable::deletePartition(Report& report, const Partition& partition)
{
	Q_UNUSED(report);
	Q_UNUSED(partition);

	return true;
}

bool DummyPartitionTable::updateGeometry(Report& report, const Partition& partition, qint64 sector_start, qint64 sector_end)
{
	Q_UNUSED(report);
	Q_UNUSED(partition);
	Q_UNUSED(sector_start);
	Q_UNUSED(sector_end);

	return true;
}

bool DummyPartitionTable::clobberFileSystem(Report& report, const Partition& partition)
{
	Q_UNUSED(report);
	Q_UNUSED(partition);

	return true;
}

bool DummyPartitionTable::resizeFileSystem(Report& report, const Partition& partition, qint64 newLength)
{
	Q_UNUSED(report);
	Q_UNUSED(partition);
	Q_UNUSED(newLength);

	return true;
}

FileSystem::Type DummyPartitionTable::detectFileSystemBySector(Report& report, const Device& device, qint64 sector)
{
	Q_UNUSED(report);
	Q_UNUSED(device);
	Q_UNUSED(sector);

	FileSystem::Type rval = FileSystem::Unknown;
	return rval;
}

bool DummyPartitionTable::setPartitionSystemType(Report& report, const Partition& partition)
{
	Q_UNUSED(report);
	Q_UNUSED(partition);

	return true;
}
