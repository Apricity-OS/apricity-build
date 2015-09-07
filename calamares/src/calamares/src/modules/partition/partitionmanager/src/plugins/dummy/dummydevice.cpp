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

#include "plugins/dummy/dummydevice.h"
#include "plugins/dummy/dummypartitiontable.h"

#include "core/partitiontable.h"

#include "util/globallog.h"
#include "util/report.h"

DummyDevice::DummyDevice(const QString& device_node) :
	CoreBackendDevice(device_node)
{
}

DummyDevice::~DummyDevice()
{
}

bool DummyDevice::open()
{
	return true;
}

bool DummyDevice::openExclusive()
{
	return true;
}

bool DummyDevice::close()
{
	return true;
}

CoreBackendPartitionTable* DummyDevice::openPartitionTable()
{
	CoreBackendPartitionTable* ptable = new DummyPartitionTable();

	if (ptable == NULL || !ptable->open())
	{
		delete ptable;
		ptable = NULL;
	}

	return ptable;
}

bool DummyDevice::createPartitionTable(Report& report, const PartitionTable& ptable)
{
	Q_UNUSED(report);
	Q_UNUSED(ptable);

	return true;
}

bool DummyDevice::readSectors(void* buffer, qint64 offset, qint64 numSectors)
{
	Q_UNUSED(buffer);
	Q_UNUSED(offset);
	Q_UNUSED(numSectors);

	if (!isExclusive())
		return false;

	return true;
}

bool DummyDevice::writeSectors(void* buffer, qint64 offset, qint64 numSectors)
{
	Q_UNUSED(buffer);
	Q_UNUSED(offset);
	Q_UNUSED(numSectors);

	if (!isExclusive())
		return false;

	return true;
}
