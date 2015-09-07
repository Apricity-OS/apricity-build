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

#include "jobs/resizefilesystemjob.h"

#include "core/partition.h"
#include "core/device.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"
#include "backend/corebackendpartitiontable.h"

#include "fs/filesystem.h"

#include "util/report.h"
#include "util/capacity.h"

#include <QDebug>

#include <KLocalizedString>

/** Creates a new ResizeFileSystemJob
	@param d the Device the FileSystem to be resized is on
	@param p the Partition the FileSystem to be resized is on
	@param newlength the new length for the FileSystem; if -1, the FileSystem will be resized to fill the entire Partition
*/
ResizeFileSystemJob::ResizeFileSystemJob(Device& d, Partition& p, qint64 newlength) :
	Job(),
	m_Device(d),
	m_Partition(p),
	m_Maximize(newlength == -1),
	m_NewLength(isMaximizing() ? partition().length() : newlength)
{
}

qint32 ResizeFileSystemJob::numSteps() const
{
	return 100;
}

bool ResizeFileSystemJob::run(Report& parent)
{
	Q_ASSERT(partition().fileSystem().firstSector() != -1);
	Q_ASSERT(partition().fileSystem().lastSector() != -1);
	Q_ASSERT(newLength() <= partition().length());

	if (partition().fileSystem().firstSector() == -1 || partition().fileSystem().lastSector() == -1 || newLength() > partition().length())
	{
		qWarning() << "file system first sector: " << partition().fileSystem().firstSector() << ", last sector: " << partition().fileSystem().lastSector() << ", new length: " << newLength() << ", partition length: " << partition().length();
		return false;
	}

	bool rval = false;

	Report* report = jobStarted(parent);

	if (partition().fileSystem().length() == newLength())
	{
		report->line() << xi18ncp("@info/plain", "The file system on partition <filename>%2</filename> already has the requested length of 1 sector.", "The file system on partition <filename>%2</filename> already has the requested length of %1 sectors.", newLength(), partition().deviceNode());
		rval = true;
	}
	else
	{
		report->line() << i18nc("@info/plain", "Resizing file system from %1 to %2 sectors.", partition().fileSystem().length(), newLength());

		FileSystem::CommandSupportType support = (newLength() < partition().fileSystem().length()) ? partition().fileSystem().supportShrink() : partition().fileSystem().supportGrow();

		switch(support)
		{
			case FileSystem::cmdSupportBackend:
			{
				Report* childReport = report->newChild();
				childReport->line() << i18nc("@info/plain", "Resizing a %1 file system using internal backend functions.", partition().fileSystem().name());
				rval = resizeFileSystemBackend(*childReport);
				break;
			}

			case FileSystem::cmdSupportFileSystem:
			{
				const qint64 newLengthInByte = Capacity(newLength() * device().logicalSectorSize()).toInt(Capacity::Byte);
				rval = partition().fileSystem().resize(*report, partition().deviceNode(), newLengthInByte);
				break;
			}

			default:
				report->line() << xi18nc("@info/plain", "The file system on partition <filename>%1</filename> cannot be resized because there is no support for it.", partition().deviceNode());
				break;
		}

		if (rval)
			partition().fileSystem().setLastSector(partition().fileSystem().firstSector() + newLength() - 1);
	}

	jobFinished(*report, rval);

	return rval;
}

bool ResizeFileSystemJob::resizeFileSystemBackend(Report& report)
{
	bool rval = false;

	CoreBackendDevice* backendDevice = CoreBackendManager::self()->backend()->openDevice(device().deviceNode());

	if (backendDevice)
	{
		CoreBackendPartitionTable* backendPartitionTable = backendDevice->openPartitionTable();

		if (backendPartitionTable)
		{
			connect(CoreBackendManager::self()->backend(), SIGNAL(progress(int)), this, SIGNAL(progress(int)));
			rval = backendPartitionTable->resizeFileSystem(report, partition(), newLength());
			disconnect(CoreBackendManager::self()->backend(), SIGNAL(progress(int)), this, SIGNAL(progress(int)));

			if (rval)
			{
				report.line() << i18nc("@info/plain", "Successfully resized file system using internal backend functions.");
				backendPartitionTable->commit();
			}

			delete backendPartitionTable;
		}
		else
			report.line() << xi18nc("@info/plain", "Could not open partition <filename>%1</filename> while trying to resize the file system.", partition().deviceNode());

		delete backendDevice;
	}
	else
		report.line() << xi18nc("@info/plain", "Could not read geometry for partition <filename>%1</filename> while trying to resize the file system.", partition().deviceNode());

	return rval;
}

QString ResizeFileSystemJob::description() const
{
	if (isMaximizing())
		return xi18nc("@info/plain", "Maximize file system on <filename>%1</filename> to fill the partition", partition().deviceNode());

	return xi18ncp("@info/plain", "Resize file system on partition <filename>%2</filename> to 1 sector", "Resize file system on partition <filename>%2</filename> to %1 sectors", newLength(), partition().deviceNode());
}
