/***************************************************************************
 *   Copyright (C) 2008,2009 by Volker Lanz <vl@fidra.de>                  *
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

#include "jobs/copyfilesystemjob.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/copysourcedevice.h"
#include "core/copytargetdevice.h"

#include "fs/filesystem.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new CopyFileSystemJob
	@param targetdevice the Device the FileSystem is to be copied to
	@param targetpartition the Partition the FileSystem is to be copied to
	@param sourcedevice the Device the source FileSystem is on
	@param sourcepartition the Partition the source FileSystem is on
*/
CopyFileSystemJob::CopyFileSystemJob(Device& targetdevice, Partition& targetpartition, Device& sourcedevice, Partition& sourcepartition) :
	Job(),
	m_TargetDevice(targetdevice),
	m_TargetPartition(targetpartition),
	m_SourceDevice(sourcedevice),
	m_SourcePartition(sourcepartition)
{
}

qint32 CopyFileSystemJob::numSteps() const
{
	return 100;
}

bool CopyFileSystemJob::run(Report& parent)
{
	bool rval = false;

	Report* report = jobStarted(parent);

	if (targetPartition().fileSystem().length() < sourcePartition().fileSystem().length())
		report->line() << xi18nc("@info/plain", "Cannot copy file system: File system on target partition <filename>%1</filename> is smaller than the file system on source partition <filename>%2</filename>.", targetPartition().deviceNode(), sourcePartition().deviceNode());
	else if (sourcePartition().fileSystem().supportCopy() == FileSystem::cmdSupportFileSystem)
		rval = sourcePartition().fileSystem().copy(*report, targetPartition().deviceNode(), sourcePartition().deviceNode());
	else if (sourcePartition().fileSystem().supportCopy() == FileSystem::cmdSupportCore)
	{
		CopySourceDevice copySource(sourceDevice(), sourcePartition().fileSystem().firstSector(), sourcePartition().fileSystem().lastSector());
		CopyTargetDevice copyTarget(targetDevice(), targetPartition().fileSystem().firstSector(), targetPartition().fileSystem().lastSector());

		if (!copySource.open())
			report->line() << xi18nc("@info/plain", "Could not open file system on source partition <filename>%1</filename> for copying.", sourcePartition().deviceNode());
		else if (!copyTarget.open())
			report->line() << xi18nc("@info/plain", "Could not open file system on target partition <filename>%1</filename> for copying.", targetPartition().deviceNode());
		else
		{
			rval = copyBlocks(*report, copyTarget, copySource);
			report->line() << i18nc("@info/plain", "Closing device. This may take a while, especially on slow devices like Memory Sticks.");
		}
	}

	if (rval)
	{
		// set the target file system to the length of the source
		const qint64 newLastSector = targetPartition().fileSystem().firstSector() + sourcePartition().fileSystem().length() - 1;

		targetPartition().fileSystem().setLastSector(newLastSector);

		// and set a new UUID, if the target filesystem supports UUIDs
		if (targetPartition().fileSystem().supportUpdateUUID() == FileSystem::cmdSupportFileSystem)
		{
			targetPartition().fileSystem().updateUUID(*report, targetPartition().deviceNode());
			targetPartition().fileSystem().setUUID(targetPartition().fileSystem().readUUID(targetPartition().deviceNode()));
		}
	}

	if (rval)
		rval = targetPartition().fileSystem().updateBootSector(*report, targetPartition().deviceNode());

	jobFinished(*report, rval);

	return rval;
}

QString CopyFileSystemJob::description() const
{
	return xi18nc("@info/plain", "Copy file system on partition <filename>%1</filename> to partition <filename>%2</filename>", sourcePartition().deviceNode(), targetPartition().deviceNode());
}
