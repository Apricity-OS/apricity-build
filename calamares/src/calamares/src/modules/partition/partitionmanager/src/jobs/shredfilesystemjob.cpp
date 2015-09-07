/***************************************************************************
 *   Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                       *
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

#include "jobs/shredfilesystemjob.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/copysourceshred.h"
#include "core/copytargetdevice.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"

#include "util/report.h"

#include <QDebug>

#include <KLocalizedString>

/** Creates a new ShredFileSystemJob
	@param d the Device the FileSystem is on
	@param p the Partition the FileSystem is in
*/
ShredFileSystemJob::ShredFileSystemJob(Device& d, Partition& p) :
	Job(),
	m_Device(d),
	m_Partition(p)
{
}

qint32 ShredFileSystemJob::numSteps() const
{
	return 100;
}

bool ShredFileSystemJob::run(Report& parent)
{
	Q_ASSERT(device().deviceNode() == partition().devicePath());

	if (device().deviceNode() != partition().devicePath())
	{
		qWarning() << "deviceNode: " << device().deviceNode() << ", partition path: " << partition().devicePath();
		return false;
	}

	bool rval = false;

	Report* report = jobStarted(parent);

	// Again, a scope for copyTarget and copySource. See MoveFileSystemJob::run()
	{
		CopyTargetDevice copyTarget(device(), partition().fileSystem().firstSector(), partition().fileSystem().lastSector());
		CopySourceShred copySource(partition().capacity(), copyTarget.sectorSize());

		if (!copySource.open())
			report->line() << i18nc("@info/plain", "Could not open random data source to overwrite file system.");
		else if (!copyTarget.open())
			report->line() << xi18nc("@info/plain", "Could not open target partition <filename>%1</filename> to restore to.", partition().deviceNode());
		else
		{
			rval = copyBlocks(*report, copyTarget, copySource);
			report->line() << i18nc("@info/plain", "Closing device. This may take a few seconds.");
		}
	}

	jobFinished(*report, rval);

	return rval;
}

QString ShredFileSystemJob::description() const
{
	return xi18nc("@info/plain", "Shred the file system on <filename>%1</filename>", partition().deviceNode());
}
