/***************************************************************************
 *   Copyright (C) 2010 by Volker Lanz <vl@fidra.de                        *
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

#include "plugins/libparted/libparteddevice.h"
#include "plugins/libparted/libpartedpartitiontable.h"

#include "core/partitiontable.h"

#include "util/globallog.h"
#include "util/report.h"

#include <KLocalizedString>

#include <unistd.h>

LibPartedDevice::LibPartedDevice(const QString& device_node) :
	CoreBackendDevice(device_node),
	m_PedDevice(NULL)
{
}

LibPartedDevice::~LibPartedDevice()
{
	if (pedDevice())
		close();
}

bool LibPartedDevice::open()
{
	Q_ASSERT(pedDevice() == NULL);

	if (pedDevice())
		return false;

	m_PedDevice = ped_device_get(deviceNode().toLatin1().constData());

	return m_PedDevice != NULL;
}

bool LibPartedDevice::openExclusive()
{
	bool rval = open() && ped_device_open(pedDevice());

	if (rval)
		setExclusive(true);

	return rval;
}

bool LibPartedDevice::close()
{
	Q_ASSERT(pedDevice());

	if (pedDevice() && isExclusive())
	{
		ped_device_close(pedDevice());
		setExclusive(false);
	}

	m_PedDevice = NULL;
	return true;
}

CoreBackendPartitionTable* LibPartedDevice::openPartitionTable()
{
	CoreBackendPartitionTable* ptable = new LibPartedPartitionTable(pedDevice());

	if (ptable == NULL || !ptable->open())
	{
		delete ptable;
		ptable = NULL;
	}

	return ptable;
}

bool LibPartedDevice::createPartitionTable(Report& report, const PartitionTable& ptable)
{
	PedDiskType* pedDiskType = ped_disk_type_get(ptable.typeName().toLatin1().constData());

	if (pedDiskType == NULL)
	{
		report.line() << xi18nc("@info/plain", "Creating partition table failed: Could not retrieve partition table type \"%1\" for <filename>%2</filename>.", ptable.typeName(), deviceNode());
		return false;
	}

	PedDevice* dev = ped_device_get(deviceNode().toLatin1().constData());

	if (dev == NULL)
	{
		report.line() << xi18nc("@info/plain", "Creating partition table failed: Could not open backend device <filename>%1</filename>.", deviceNode());
		return false;
	}

	PedDisk* disk = ped_disk_new_fresh(dev, pedDiskType);

	if (disk == NULL)
	{
		report.line() << xi18nc("@info/plain", "Creating partition table failed: Could not create a new partition table in the backend for device <filename>%1</filename>.", deviceNode());
		return false;
	}

	return LibPartedPartitionTable::commit(disk);
}

bool LibPartedDevice::readSectors(void* buffer, qint64 offset, qint64 numSectors)
{
	if (!isExclusive())
		return false;

	return ped_device_read(pedDevice(), buffer, offset, numSectors);
}

bool LibPartedDevice::writeSectors(void* buffer, qint64 offset, qint64 numSectors)
{
	if (!isExclusive())
		return false;

	return ped_device_write(pedDevice(), buffer, offset, numSectors);
}
