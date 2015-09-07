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

#include "jobs/restorefilesystemjob.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"
#include "backend/corebackendpartitiontable.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/copysourcefile.h"
#include "core/copytargetdevice.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new RestoreFileSystemJob
	@param targetdevice the Device the FileSystem is to be restored to
	@param targetpartition the Partition the FileSystem is to be restore to
	@param filename the file name with the image file to restore
*/
RestoreFileSystemJob::RestoreFileSystemJob(Device& targetdevice, Partition& targetpartition, const QString& filename) :
	Job(),
	m_TargetDevice(targetdevice),
	m_TargetPartition(targetpartition),
	m_FileName(filename)
{
}

qint32 RestoreFileSystemJob::numSteps() const
{
	return 100;
}

bool RestoreFileSystemJob::run(Report& parent)
{
	// Restoring is file system independent because we currently have no way of
	// detecting the file system in a given image file. We cannot even find out if the
	// file the user gave us is a valid image file or just some junk.

	bool rval = false;

	Report* report = jobStarted(parent);

	// Again, a scope for copyTarget and copySource. See MoveFileSystemJob::run()
	{
		// FileSystems are restored to _partitions_, so don't use first and last sector of file system here
		CopyTargetDevice copyTarget(targetDevice(), targetPartition().firstSector(), targetPartition().lastSector());
		CopySourceFile copySource(fileName(), copyTarget.sectorSize());

		if (!copySource.open())
			report->line() << xi18nc("@info/plain", "Could not open backup file <filename>%1</filename> to restore from.", fileName());
		else if (!copyTarget.open())
			report->line() << xi18nc("@info/plain", "Could not open target partition <filename>%1</filename> to restore to.", targetPartition().deviceNode());
		else
		{
			rval = copyBlocks(*report, copyTarget, copySource);

			if (rval)
			{
				// create a new file system for what was restored with the length of the image file
				const qint64 newLastSector = targetPartition().firstSector() + copySource.length() - 1;

				CoreBackendDevice* backendDevice = CoreBackendManager::self()->backend()->openDevice(targetDevice().deviceNode());

				FileSystem::Type t = FileSystem::Unknown;

				if (backendDevice)
				{
					CoreBackendPartitionTable* backendPartitionTable = backendDevice->openPartitionTable();

					if (backendPartitionTable)
						t = backendPartitionTable->detectFileSystemBySector(*report, targetDevice(), targetPartition().firstSector());
				}

				FileSystem* fs = FileSystemFactory::create(t, targetPartition().firstSector(), newLastSector);

				targetPartition().deleteFileSystem();
				targetPartition().setFileSystem(fs);
			}

			report->line() << i18nc("@info/plain", "Closing device. This may take a few seconds.");
		}
	}

	jobFinished(*report, rval);

	return rval;
}

QString RestoreFileSystemJob::description() const
{
	return xi18nc("@info/plain", "Restore the file system from file <filename>%1</filename> to partition <filename>%2</filename>", fileName(), targetPartition().deviceNode());
}
