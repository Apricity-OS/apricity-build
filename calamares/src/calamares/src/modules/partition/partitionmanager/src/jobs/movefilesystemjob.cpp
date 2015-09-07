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

#include "jobs/movefilesystemjob.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/copysourcedevice.h"
#include "core/copytargetdevice.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new MoveFileSystemJob
	@param d the Device the Partition to move is on
	@param p the Partition to move
	@param newstart the new start sector for the Partition
*/
MoveFileSystemJob::MoveFileSystemJob(Device& d, Partition& p, qint64 newstart) :
	Job(),
	m_Device(d),
	m_Partition(p),
	m_NewStart(newstart)
{
}

qint32 MoveFileSystemJob::numSteps() const
{
	return 100;
}

bool MoveFileSystemJob::run(Report& parent)
{
	bool rval = false;

	Report* report = jobStarted(parent);

	// A scope for moveSource and moveTarget, so CopyTargetDevice's dtor runs before we
	// say we're finished: The CopyTargetDevice dtor asks the backend to close the device
	// and that may take a while.
	{
		CopySourceDevice moveSource(device(), partition().fileSystem().firstSector(), partition().fileSystem().lastSector());
		CopyTargetDevice moveTarget(device(), newStart(), newStart() + partition().fileSystem().length());

		if (!moveSource.open())
			report->line() << xi18nc("@info/plain", "Could not open file system on partition <filename>%1</filename> for moving.", partition().deviceNode());
		else if (!moveTarget.open())
			report->line() << xi18nc("@info/plain", "Could not create target for moving file system on partition <filename>%1</filename>.", partition().deviceNode());
		else
		{
			rval = copyBlocks(*report, moveTarget, moveSource);

			if (rval)
			{
				const qint64 savedLength = partition().fileSystem().length() - 1;
				partition().fileSystem().setFirstSector(newStart());
				partition().fileSystem().setLastSector(newStart() + savedLength);
			}
			else if (!rollbackCopyBlocks(*report, moveTarget, moveSource))
				report->line() << xi18nc("@info/plain", "Rollback for file system on partition <filename>%1</filename> failed.", partition().deviceNode());

			report->line() << i18nc("@info/plain", "Closing device. This may take a few seconds.");
		}
	}

	if (rval)
		rval = partition().fileSystem().updateBootSector(*report, partition().deviceNode());

	jobFinished(*report, rval);

	return rval;
}

QString MoveFileSystemJob::description() const
{
	return xi18nc("@info/plain", "Move the file system on partition <filename>%1</filename> to sector %2", partition().deviceNode(), newStart());
}
