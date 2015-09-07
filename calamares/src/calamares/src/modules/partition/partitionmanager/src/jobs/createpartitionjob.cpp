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

#include "jobs/createpartitionjob.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"
#include "backend/corebackendpartitiontable.h"

#include "core/partition.h"
#include "core/device.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new CreatePartitionJob
	@param d the Device the Partition to be created will be on
	@param p the Partition to create
*/
CreatePartitionJob::CreatePartitionJob(Device& d, Partition& p) :
	Job(),
	m_Device(d),
	m_Partition(p)
{
}

bool CreatePartitionJob::run(Report& parent)
{
	Q_ASSERT(partition().devicePath() == device().deviceNode());

	bool rval = false;

	Report* report = jobStarted(parent);

	CoreBackendDevice* backendDevice = CoreBackendManager::self()->backend()->openDevice(device().deviceNode());

	if (backendDevice)
	{
		CoreBackendPartitionTable* backendPartitionTable = backendDevice->openPartitionTable();

		if (backendPartitionTable)
		{
			QString partitionPath = backendPartitionTable->createPartition(*report, partition());

			if (partitionPath != QString())
			{
				rval = true;
				partition().setPartitionPath(partitionPath);
				partition().setState(Partition::StateNone);
				backendPartitionTable->commit();
			}
			else
				report->line() << xi18nc("@info/plain", "Failed to add partition <filename>%1</filename> to device <filename>%2</filename>.", partition().deviceNode(), device().deviceNode());

			delete backendPartitionTable;
		}
		else
			report->line() << xi18nc("@info/plain", "Could not open partition table on device <filename>%1</filename> to create new partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());

		delete backendDevice;
	}
	else
		report->line() << xi18nc("@info/plain", "Could not open device <filename>%1</filename> to create new partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());

	jobFinished(*report, rval);

	return rval;
}

QString CreatePartitionJob::description() const
{
	if (partition().number() > 0)
		return xi18nc("@info/plain", "Create new partition <filename>%1</filename>", partition().deviceNode());

	return xi18nc("@info/plain", "Create new partition on device <filename>%1</filename>", device().deviceNode());
}
