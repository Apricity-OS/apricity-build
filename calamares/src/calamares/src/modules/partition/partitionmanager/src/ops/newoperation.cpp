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

#include "ops/newoperation.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/partitionnode.h"

#include "jobs/createpartitionjob.h"
#include "jobs/createfilesystemjob.h"
#include "jobs/setfilesystemlabeljob.h"
#include "jobs/setpartflagsjob.h"
#include "jobs/checkfilesystemjob.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"

#include "util/capacity.h"

#include <QString>

#include <KLocalizedString>

/** Creates a new NewOperation.
	@param d the Device to create a new Partition on
	@param p pointer to the new Partition to create. May not be NULL.
*/
NewOperation::NewOperation(Device& d, Partition* p) :
	Operation(),
	m_TargetDevice(d),
	m_NewPartition(p),
	m_CreatePartitionJob(new CreatePartitionJob(targetDevice(), newPartition())),
	m_CreateFileSystemJob(NULL),
	m_SetPartFlagsJob(NULL),
	m_SetFileSystemLabelJob(NULL),
	m_CheckFileSystemJob(NULL)
{
	addJob(createPartitionJob());

	const FileSystem& fs = newPartition().fileSystem();

	if (fs.type() != FileSystem::Extended)
	{
		// It would seem tempting to skip the CreateFileSystemJob or the
		// SetFileSystemLabelJob if either has nothing to do (unformatted FS or
		// empty label). However, the user might later on decide to change FS or
		// label. The operation stack will merge these operations with this one here
		// and if the jobs don't exist things will break.

		m_CreateFileSystemJob = new CreateFileSystemJob(targetDevice(), newPartition());
		addJob(createFileSystemJob());

		if (fs.type() == FileSystem::Lvm2_PV)
		{
			m_SetPartFlagsJob = new SetPartFlagsJob(targetDevice(), newPartition(), PartitionTable::FlagLvm);
			addJob(setPartFlagsJob());
		}

		m_SetFileSystemLabelJob = new SetFileSystemLabelJob(newPartition(), fs.label());
		addJob(setLabelJob());

		m_CheckFileSystemJob = new CheckFileSystemJob(newPartition());
		addJob(checkJob());
	}
}

NewOperation::~NewOperation()
{
	if (status() == StatusPending)
		delete m_NewPartition;
}

bool NewOperation::targets(const Device& d) const
{
	return d == targetDevice();
}

bool NewOperation::targets(const Partition& p) const
{
	return p == newPartition();
}

void NewOperation::preview()
{
	insertPreviewPartition(targetDevice(), newPartition());
}

void NewOperation::undo()
{
	removePreviewPartition(targetDevice(), newPartition());
}

QString NewOperation::description() const
{
	return xi18nc("@info/plain", "Create a new partition (%1, %2) on <filename>%3</filename>", Capacity::formatByteSize(newPartition().capacity()), newPartition().fileSystem().name(), targetDevice().deviceNode());
}

/** Can a Partition be created somewhere?
	@param p the Partition where a new Partition is to be created, may be NULL
	@return true if a new Partition can be created in @p p
 */
bool NewOperation::canCreateNew(const Partition* p)
{
	return p != NULL && p->roles().has(PartitionRole::Unallocated);
}

Partition* NewOperation::createNew(const Partition& cloneFrom)
{
	Partition* p = new Partition(cloneFrom);

	p->deleteFileSystem();
	p->setFileSystem(FileSystemFactory::create(FileSystem::defaultFileSystem(), p->firstSector(), p->lastSector()));
	p->setState(Partition::StateNew);
	p->setPartitionPath(QString());

	return p;
}
