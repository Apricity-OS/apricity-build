/***************************************************************************
 *   Copyright (C) 2008,2011 by Volker Lanz <vl@fidra.de>                  *
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

#include "jobs/createfilesystemjob.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"
#include "backend/corebackendpartitiontable.h"

#include "core/device.h"
#include "core/partition.h"

#include "fs/filesystem.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new CreateFileSystemJob
	@param p the Partition the FileSystem to create is on
*/
CreateFileSystemJob::CreateFileSystemJob(Device& d, Partition& p) :
	Job(),
	m_Device(d),
	m_Partition(p)
{
}

bool CreateFileSystemJob::run(Report& parent)
{
	bool rval = false;

	Report* report = jobStarted(parent);

	if (partition().fileSystem().supportCreate() == FileSystem::cmdSupportFileSystem)
	{
		if (partition().fileSystem().create(*report, partition().deviceNode()))
		{
			CoreBackendDevice* backendDevice = CoreBackendManager::self()->backend()->openDevice(device().deviceNode());

			if (backendDevice)
			{
				CoreBackendPartitionTable* backendPartitionTable = backendDevice->openPartitionTable();

				if (backendPartitionTable)
				{
					if (backendPartitionTable->setPartitionSystemType(*report, partition()))
					{
						rval = true;
						backendPartitionTable->commit();
					}
					else
						report->line() << xi18nc("@info/plain", "Failed to set the system type for the file system on partition <filename>%1</filename>.", partition().deviceNode());

					delete backendPartitionTable;
				}
				else
					report->line() << xi18nc("@info/plain", "Could not open partition table on device <filename>%1</filename> to set the system type for partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());

				delete backendDevice;
			}
			else
				report->line() << xi18nc("@info/plain", "Could not open device <filename>%1</filename> to set the system type for partition <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());
		}
	}

	jobFinished(*report, rval);

	return rval;
}

QString CreateFileSystemJob::description() const
{
	return xi18nc("@info/plain", "Create file system <filename>%1</filename> on partition <filename>%2</filename>", partition().fileSystem().name(), partition().deviceNode());
}
