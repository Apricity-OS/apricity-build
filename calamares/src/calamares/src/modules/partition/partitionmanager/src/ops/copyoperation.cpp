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

#include "ops/copyoperation.h"

#include "core/partition.h"
#include "core/device.h"

#include "jobs/createpartitionjob.h"
#include "jobs/deletepartitionjob.h"
#include "jobs/checkfilesystemjob.h"
#include "jobs/copyfilesystemjob.h"
#include "jobs/resizefilesystemjob.h"

#include "fs/filesystemfactory.h"

#include "util/capacity.h"
#include "util/report.h"

#include <QDebug>
#include <QString>

#include <KLocalizedString>

/** Creates a new CopyOperation.
	@param targetdevice the Device to copy the Partition to
	@param copiedpartition pointer to the new Partition object on the target Device. May not be NULL.
	@param sourcedevice the Device where to copy from
	@param sourcepartition pointer to the Partition to copy from. May not be NULL.
*/
CopyOperation::CopyOperation(Device& targetdevice, Partition* copiedpartition, Device& sourcedevice, Partition* sourcepartition) :
	Operation(),
	m_TargetDevice(targetdevice),
	m_CopiedPartition(copiedpartition),
	m_SourceDevice(sourcedevice),
	m_SourcePartition(sourcepartition),
	m_OverwrittenPartition(NULL),
	m_MustDeleteOverwritten(false),
	m_CheckSourceJob(NULL),
	m_CreatePartitionJob(NULL),
	m_CopyFSJob(NULL),
	m_CheckTargetJob(NULL),
	m_MaximizeJob(NULL),
	m_Description(updateDescription())
{
	Q_ASSERT(targetDevice().partitionTable());

	Partition* dest = targetDevice().partitionTable()->findPartitionBySector(copiedPartition().firstSector(), PartitionRole(PartitionRole::Primary | PartitionRole::Logical | PartitionRole::Unallocated));

	Q_ASSERT(dest);

	if (dest == NULL)
		qWarning() << "destination partition not found at sector " << copiedPartition().firstSector();

	if (dest && !dest->roles().has(PartitionRole::Unallocated))
	{
		copiedPartition().setLastSector(dest->lastSector());
		setOverwrittenPartition(dest);
	}

	addJob(m_CheckSourceJob = new CheckFileSystemJob(sourcePartition()));

	if (overwrittenPartition() == NULL)
		addJob(m_CreatePartitionJob = new CreatePartitionJob(targetDevice(), copiedPartition()));

	addJob(m_CopyFSJob = new CopyFileSystemJob(targetDevice(), copiedPartition(), sourceDevice(), sourcePartition()));
	addJob(m_CheckTargetJob = new CheckFileSystemJob(copiedPartition()));
	addJob(m_MaximizeJob = new ResizeFileSystemJob(targetDevice(), copiedPartition()));
}

CopyOperation::~CopyOperation()
{
	if (status() == StatusPending)
		delete m_CopiedPartition;

	if (status() == StatusFinishedSuccess || status() == StatusFinishedWarning || status() == StatusError)
		cleanupOverwrittenPartition();
}

bool CopyOperation::targets(const Device& d) const
{
	return d == targetDevice();
}

bool CopyOperation::targets(const Partition& p) const
{
	return p == copiedPartition();
}

void CopyOperation::preview()
{
	if (overwrittenPartition())
		removePreviewPartition(targetDevice(), *overwrittenPartition());

	insertPreviewPartition(targetDevice(), copiedPartition());
}

void CopyOperation::undo()
{
	removePreviewPartition(targetDevice(), copiedPartition());

	if (overwrittenPartition())
		insertPreviewPartition(targetDevice(), *overwrittenPartition());
}

bool CopyOperation::execute(Report& parent)
{
	bool rval = false;
	bool warning = false;

	Report* report = parent.newChild(description());

	// check the source first
	if ((rval = checkSourceJob()->run(*report)))
	{
		// At this point, if the target partition is to be created and not overwritten, it
		// will still have the wrong device path (the one of the source device). We need
		// to adjust that before we're creating it.
		copiedPartition().setDevicePath(targetDevice().deviceNode());

		// either we have no partition to create (because we're overwriting) or creating
		// must be successful
		if (!createPartitionJob() || (rval = createPartitionJob()->run(*report)))
		{
			// set the state of the target partition from StateCopy to StateNone or checking
			// it will fail (because its deviceNode() will still be "Copy of sdXn"). This is
			// only required for overwritten partitions, but doesn't hurt in any case.
			copiedPartition().setState(Partition::StateNone);

			// if we have overwritten a partition, reset device path and number
			if (overwrittenPartition())
			{
				copiedPartition().setDevicePath(overwrittenPartition()->devicePath());
				copiedPartition().setPartitionPath(overwrittenPartition()->devicePath());
			}

			// now run the copy job itself
			if ((rval = copyFSJob()->run(*report)))
			{
				// and if the copy job succeeded, check the target
				if ((rval = checkTargetJob()->run(*report)))
				{
					// ok, everything went well
					rval = true;

					// if maximizing doesn't work, just warn the user, don't fail
					if (!maximizeJob()->run(*report))
					{
						report->line() << xi18nc("@info/plain", "Warning: Maximizing file system on target partition <filename>%1</filename> to the size of the partition failed.", copiedPartition().deviceNode());
						warning = true;
					}
				}
				else
					report->line() << xi18nc("@info/plain", "Checking target partition <filename>%1</filename> after copy failed.", copiedPartition().deviceNode());
			}
			else
			{
				if (createPartitionJob())
				{
					DeletePartitionJob deleteJob(targetDevice(), copiedPartition());
					deleteJob.run(*report);
				}

				report->line() << i18nc("@info/plain", "Copying source to target partition failed.");
			}
		}
		else
			report->line() << i18nc("@info/plain", "Creating target partition for copying failed.");
	}
	else
		report->line() << xi18nc("@info/plain", "Checking source partition <filename>%1</filename> failed.", sourcePartition().deviceNode());

	if (rval)
		setStatus(warning ? StatusFinishedWarning : StatusFinishedSuccess);
	else
		setStatus(StatusError);

	report->setStatus(i18nc("@info/plain status (success, error, warning...) of operation", "%1: %2", description(), statusText()));

	return rval;
}

QString CopyOperation::updateDescription() const
{
	if (overwrittenPartition())
	{
		if (copiedPartition().length() == overwrittenPartition()->length())
			return xi18nc("@info/plain", "Copy partition <filename>%1</filename> (%2, %3) to <filename>%4</filename> (%5, %6)",
				sourcePartition().deviceNode(),
				Capacity::formatByteSize(sourcePartition().capacity()),
				sourcePartition().fileSystem().name(),
				overwrittenPartition()->deviceNode(),
				Capacity::formatByteSize(overwrittenPartition()->capacity()),
				overwrittenPartition()->fileSystem().name()
			);

		return xi18nc("@info/plain", "Copy partition <filename>%1</filename> (%2, %3) to <filename>%4</filename> (%5, %6) and grow it to %7",
			sourcePartition().deviceNode(),
			Capacity::formatByteSize(sourcePartition().capacity()),
			sourcePartition().fileSystem().name(),
			overwrittenPartition()->deviceNode(),
			Capacity::formatByteSize(overwrittenPartition()->capacity()),
			overwrittenPartition()->fileSystem().name(),
			Capacity::formatByteSize(copiedPartition().capacity())
		);
	}

	if (copiedPartition().length() == sourcePartition().length())
		return xi18nc("@info/plain", "Copy partition <filename>%1</filename> (%2, %3) to unallocated space (starting at %4) on <filename>%5</filename>",
			sourcePartition().deviceNode(),
			Capacity::formatByteSize(sourcePartition().capacity()),
			sourcePartition().fileSystem().name(),
			Capacity::formatByteSize(copiedPartition().firstSector() * targetDevice().logicalSectorSize()),
			targetDevice().deviceNode()
		);

	return xi18nc("@info/plain", "Copy partition <filename>%1</filename> (%2, %3) to unallocated space (starting at %4) on <filename>%5</filename> and grow it to %6",
		sourcePartition().deviceNode(),
		Capacity::formatByteSize(sourcePartition().capacity()),
		sourcePartition().fileSystem().name(),
		Capacity::formatByteSize(copiedPartition().firstSector() * targetDevice().logicalSectorSize()),
		targetDevice().deviceNode(),
		Capacity::formatByteSize(copiedPartition().capacity())
	);
}

void CopyOperation::setOverwrittenPartition(Partition* p)
{
	// this code is also in RestoreOperation.

	cleanupOverwrittenPartition();
	m_OverwrittenPartition = p;

	// If the overwritten partition has no other operation that owns it (e.g., an OperationNew or
	// an OperationRestore), we're the new owner. So remember that, because after the operations all
	// have executed and we're asked to clean up after ourselves, the state of the overwritten partition
	// might have changed: If it was a new one and the NewOperation has successfully run, the state will
	// then be StateNone.
	m_MustDeleteOverwritten = (p && p->state() == Partition::StateNone);
}

void CopyOperation::cleanupOverwrittenPartition()
{
	if (mustDeleteOverwritten())
	{
		delete overwrittenPartition();
		m_OverwrittenPartition = NULL;
	}
}

/** Creates a new copied Partition.
	@param target the target Partition to copy to (may be unallocated)
	@param source the source Partition to copy
	@return pointer to the newly created Partition object
*/
Partition* CopyOperation::createCopy(const Partition& target, const Partition& source)
{
	Partition* p = target.roles().has(PartitionRole::Unallocated) ? new Partition(source) : new Partition(target);

	p->setDevicePath(source.devicePath());
	p->setPartitionPath(source.partitionPath());
	p->setState(Partition::StateCopy);

	p->deleteFileSystem();
	p->setFileSystem(FileSystemFactory::create(source.fileSystem()));

	p->fileSystem().setFirstSector(p->firstSector());
	p->fileSystem().setLastSector(p->lastSector());

	p->setFlags(PartitionTable::FlagNone);

	return p;
}

/** Can a Partition be copied?
	@param p the Partition in question, may be NULL.
	@return true if @p p can be copied.
 */
bool CopyOperation::canCopy(const Partition* p)
{
	if (p == NULL)
		return false;

	if (p->isMounted())
		return false;

	// Normally, copying partitions that have not been written to disk yet should
	// be forbidden here. The operation stack, however, will take care of these
	// problematic cases when pushing the CopyOperation onto the stack.

	return p->fileSystem().supportCopy() != FileSystem::cmdSupportNone;
}

/** Can a Partition be pasted on another one?
	@param p the Partition to be pasted to, may be NULL
	@param source the Partition to be pasted, may be NULL
	@return true if @p source can be pasted on @p p
 */
bool CopyOperation::canPaste(const Partition* p, const Partition* source)
{
	if (p == NULL || source == NULL)
		return false;

	if (p->isMounted())
		return false;

	if (p->roles().has(PartitionRole::Extended))
		return false;

	if (p == source)
		return false;

	if (source->length() > p->length())
		return false;

	if (!p->roles().has(PartitionRole::Unallocated) && p->capacity() > source->fileSystem().maxCapacity())
		return false;

	return true;
}
