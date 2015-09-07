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

#include "jobs/createpartitiontablejob.h"

#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"
#include "backend/corebackend.h"

#include "core/device.h"
#include "core/partitiontable.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new CreatePartitionTableJob
	@param d the Device a new PartitionTable is to be created on
*/
CreatePartitionTableJob::CreatePartitionTableJob(Device& d) :
	Job(),
	m_Device(d)
{
}

bool CreatePartitionTableJob::run(Report& parent)
{
	bool rval = false;

	Report* report = jobStarted(parent);

	CoreBackendDevice* backendDevice = CoreBackendManager::self()->backend()->openDevice(device().deviceNode());

	if (backendDevice != NULL)
	{
		Q_ASSERT(device().partitionTable());

		rval = backendDevice->createPartitionTable(*report, *device().partitionTable());

		delete backendDevice;
	}
	else
		report->line() << xi18nc("@info/plain", "Creating partition table failed: Could not open device <filename>%1</filename>.", device().deviceNode());

	jobFinished(*report, rval);

	return rval;
}

QString CreatePartitionTableJob::description() const
{
	return xi18nc("@info/plain", "Create new partition table on device <filename>%1</filename>", device().deviceNode());
}
