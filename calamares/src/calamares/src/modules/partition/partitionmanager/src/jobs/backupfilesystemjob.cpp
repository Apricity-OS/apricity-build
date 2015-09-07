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

#include "jobs/backupfilesystemjob.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/copysourcedevice.h"
#include "core/copytargetfile.h"

#include "fs/filesystem.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new BackupFileSystemJob
	@param sourcedevice the device the FileSystem to back up is on
	@param sourcepartition the Partition the FileSystem to back up is on
	@param filename name of the file to backup to
*/
BackupFileSystemJob::BackupFileSystemJob(Device& sourcedevice, Partition& sourcepartition, const QString& filename) :
	Job(),
	m_SourceDevice(sourcedevice),
	m_SourcePartition(sourcepartition),
	m_FileName(filename)
{
}

qint32 BackupFileSystemJob::numSteps() const
{
	return 100;
}

bool BackupFileSystemJob::run(Report& parent)
{
	bool rval = false;

	Report* report = jobStarted(parent);

	if (sourcePartition().fileSystem().supportBackup() == FileSystem::cmdSupportFileSystem)
		rval = sourcePartition().fileSystem().backup(*report, sourceDevice(), sourcePartition().deviceNode(), fileName());
	else if (sourcePartition().fileSystem().supportBackup() == FileSystem::cmdSupportCore)
	{
		CopySourceDevice copySource(sourceDevice(), sourcePartition().fileSystem().firstSector(), sourcePartition().fileSystem().lastSector());
		CopyTargetFile copyTarget(fileName(), sourceDevice().logicalSectorSize());

		if (!copySource.open())
			report->line() << xi18nc("@info/plain", "Could not open file system on source partition <filename>%1</filename> for backup.", sourcePartition().deviceNode());
		else if (!copyTarget.open())
			report->line() << xi18nc("@info/plain", "Could not create backup file <filename>%1</filename>.", fileName());
		else
			rval = copyBlocks(*report, copyTarget, copySource);
	}

	jobFinished(*report, rval);

	return rval;
}

QString BackupFileSystemJob::description() const
{
	return xi18nc("@info/plain", "Back up file system on partition <filename>%1</filename> to <filename>%2</filename>", sourcePartition().deviceNode(), fileName());
}
