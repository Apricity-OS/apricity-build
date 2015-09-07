/***************************************************************************
 *   Copyright (C) 2012 by Volker Lanz <vl@fidra.de>                       *
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

#include "jobs/deletefilesystemjob.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"
#include "backend/corebackendpartitiontable.h"

#include "core/partition.h"
#include "core/device.h"

#include "util/report.h"

#include <QDebug>

#include <KLocalizedString>

/** Creates a new DeleteFileSystemJob
	@param d the Device the FileSystem to delete is on
	@param p the Partition the FileSystem to delete is on
*/
DeleteFileSystemJob::DeleteFileSystemJob(Device& d, Partition& p) :
	Job(),
	m_Device(d),
	m_Partition(p)
{
}

bool DeleteFileSystemJob::run(Report& parent)
{
	Q_ASSERT(device().deviceNode() == partition().devicePath());

	if (device().deviceNode() != partition().devicePath())
	{
		qWarning() << "deviceNode: " << device().deviceNode() << ", partition path: " << partition().devicePath();
		return false;
	}

	bool rval = false;

	Report* report = jobStarted(parent);

	if (partition().roles().has(PartitionRole::Extended))
		rval = true;
	else
	{
		if (!partition().fileSystem().remove(*report, partition().deviceNode()))
		{
			jobFinished(*report, rval);
			return false;
		}

		CoreBackendDevice* backendDevice = CoreBackendManager::self()->backend()->openDevice(device().deviceNode());

		if (backendDevice)
		{
			CoreBackendPartitionTable* backendPartitionTable = backendDevice->openPartitionTable();

			if (backendPartitionTable)
			{
				rval = backendPartitionTable->clobberFileSystem(*report, partition());

				if (!rval)
					report->line() << xi18nc("@info/plain", "Could not delete file system on <filename>%1</filename>.", partition().deviceNode());
				else
					backendPartitionTable->commit();

				delete backendPartitionTable;

			}
			else
				report->line() << xi18nc("@info/plain", "Could not open partition table on device <filename>%1</filename> to delete file system on <filename>%2</filename>.", device().deviceNode(), partition().deviceNode());

			delete backendDevice;
		}
		else
			report->line() << xi18nc("@info/plain", "Could not delete file system signature for partition <filename>%1</filename>: Failed to open device <filename>%2</filename>.", partition().deviceNode(), device().deviceNode());
	}

	jobFinished(*report, rval);

	return rval;
}

QString DeleteFileSystemJob::description() const
{
	return xi18nc("@info/plain", "Delete file system on <filename>%1</filename>", partition().deviceNode());
}
