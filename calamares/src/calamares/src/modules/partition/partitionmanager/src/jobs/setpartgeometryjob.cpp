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

#include "jobs/setpartgeometryjob.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"
#include "backend/corebackendpartitiontable.h"

#include "core/partition.h"
#include "core/device.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new SetPartGeometryJob
	@param d the Device the Partition whose geometry is to be set is on
	@param p the Partition whose geometry is to be set
	@param newstart the new start sector for the Partition
	@param newlength the new length for the Partition

	@todo Wouldn't it be better to have newfirst (new first sector) and newlast (new last sector) as args instead?
	Having a length here doesn't seem to be very consistent with the rest of the app, right?
*/
SetPartGeometryJob::SetPartGeometryJob(Device& d, Partition& p, qint64 newstart, qint64 newlength) :
	Job(),
	m_Device(d),
	m_Partition(p),
	m_NewStart(newstart),
	m_NewLength(newlength)
{
}

bool SetPartGeometryJob::run(Report& parent)
{
	bool rval = false;

	Report* report = jobStarted(parent);

	CoreBackendDevice* backendDevice = CoreBackendManager::self()->backend()->openDevice(device().deviceNode());

	if (backendDevice)
	{
		CoreBackendPartitionTable* backendPartitionTable = backendDevice->openPartitionTable();

		if (backendPartitionTable)
		{
			rval = backendPartitionTable->updateGeometry(*report, partition(), newStart(), newStart() + newLength() - 1);

			if (rval)
			{
				partition().setFirstSector(newStart());
				partition().setLastSector(newStart() + newLength() - 1);
				backendPartitionTable->commit();
			}

			delete backendPartitionTable;
		}

		delete backendDevice;
	}
	else
		report->line() << xi18nc("@info/plain", "Could not open device <filename>%1</filename> while trying to resize/move partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());

	jobFinished(*report, rval);

	return rval;
}

QString SetPartGeometryJob::description() const
{
	return xi18nc("@info/plain", "Set geometry of partition <filename>%1</filename>: Start sector: %2, length: %3", partition().deviceNode(), newStart(), newLength());
}
