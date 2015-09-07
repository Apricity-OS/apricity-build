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

#include "backend/corebackend.h"

#include "core/device.h"
#include "core/partitiontable.h"

#include "util/globallog.h"

#include <QDebug>

#include <config.h>

class CoreBackend::CoreBackendPrivate
{
	public:
		CoreBackendPrivate(CoreBackend& cb) : coreBackend(cb) {}

	private:
		CoreBackend& coreBackend;
};

CoreBackend::CoreBackend() :
	m_AboutData(NULL),
	d(new CoreBackendPrivate(*this))
{
}

CoreBackend::~CoreBackend()
{
	delete d;
}

void CoreBackend::emitProgress(int i)
{
	emit progress(i);
}

void CoreBackend::emitScanProgress(const QString& device_node, int i)
{
	emit scanProgress(device_node, i);
}

void CoreBackend::setPartitionTableForDevice(Device& d, PartitionTable* p)
{
	d.setPartitionTable(p);
}

void CoreBackend::setPartitionTableMaxPrimaries(PartitionTable& p, qint32 max_primaries)
{
	p.setMaxPrimaries(max_primaries);
}
