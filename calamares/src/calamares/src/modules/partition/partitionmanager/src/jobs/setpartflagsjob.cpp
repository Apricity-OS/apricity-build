/***************************************************************************
 *   Copyright (C) 2008,2010 by Volker Lanz <vl@fidra.de>                  *
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

#include "jobs/setpartflagsjob.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"
#include "backend/corebackendpartition.h"
#include "backend/corebackendpartitiontable.h"

#include "core/device.h"
#include "core/partition.h"
#include "core/partitionrole.h"
#include "core/partitiontable.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new SetPartFlagsJob
	@param d the Device the Partition whose flags are to be set is on
	@param p the Partition whose flags are to be set
	@param flags the new flags for the Partition
*/
SetPartFlagsJob::SetPartFlagsJob(Device& d, Partition& p, PartitionTable::Flags flags) :
	Job(),
	m_Device(d),
	m_Partition(p),
	m_Flags(flags)
{
}

qint32 SetPartFlagsJob::numSteps() const
{
	return PartitionTable::flagList().size();
}

bool SetPartFlagsJob::run(Report& parent)
{
	bool rval = true;

	Report* report = jobStarted(parent);

	CoreBackendDevice* backendDevice = CoreBackendManager::self()->backend()->openDevice(device().deviceNode());

	if (backendDevice)
	{
		CoreBackendPartitionTable* backendPartitionTable = backendDevice->openPartitionTable();

		if (backendPartitionTable)
		{
			CoreBackendPartition* backendPartition = (partition().roles().has(PartitionRole::Extended))
				? backendPartitionTable->getExtendedPartition()
				: backendPartitionTable->getPartitionBySector(partition().firstSector());

			if (backendPartition)
			{
				quint32 count = 0;

				foreach(const PartitionTable::Flag& f, PartitionTable::flagList())
				{
					emit progress(++count);

					const bool state = (flags() & f) ? true : false;

					if (!backendPartition->setFlag(*report, f, state))
					{
						report->line() << xi18nc("@info/plain", "There was an error setting flag %1 for partition <filename>%2</filename> to state %3.", PartitionTable::flagName(f), partition().deviceNode(), state ? i18nc("@info/plain flag turned on, active", "on") : i18nc("@info/plain flag turned off, inactive", "off"));

						rval = false;
					}
				}

				delete backendPartition;
			}
			else
				report->line() << xi18nc("@info/plain", "Could not find partition <filename>%1</filename> on device <filename>%2</filename> to set partition flags.", partition().deviceNode(), device().deviceNode());

			if (rval)
				backendPartitionTable->commit();

			delete backendPartitionTable;
		}
		else
			report->line() << xi18nc("@info/plain", "Could not open partition table on device <filename>%1</filename> to set partition flags for partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());

		delete backendDevice;
	}
	else
		report->line() << xi18nc("@info/plain", "Could not open device <filename>%1</filename> to set partition flags for partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());

	if (rval)
		partition().setFlags(flags());

	jobFinished(*report, rval);

	return rval;
}

QString SetPartFlagsJob::description() const
{
	if (PartitionTable::flagNames(flags()).size() == 0)
		return xi18nc("@info/plain", "Clear flags for partition <filename>%1</filename>", partition().deviceNode());

	return xi18nc("@info/plain", "Set the flags for partition <filename>%1</filename> to \"%2\"", partition().deviceNode(), PartitionTable::flagNames(flags()).join(QStringLiteral(",")));
}
