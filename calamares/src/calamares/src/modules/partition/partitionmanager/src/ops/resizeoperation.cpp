/***************************************************************************
 *   Copyright (C) 2008,2012 by Volker Lanz <vl@fidra.de>                  *
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

#include "ops/resizeoperation.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/partitiontable.h"
#include "core/copysourcedevice.h"
#include "core/copytargetdevice.h"

#include "jobs/checkfilesystemjob.h"
#include "jobs/setpartgeometryjob.h"
#include "jobs/resizefilesystemjob.h"
#include "jobs/movefilesystemjob.h"

#include "fs/filesystem.h"

#include "util/capacity.h"
#include "util/report.h"

#include <QDebug>
#include <QString>

#include <KLocalizedString>

/** Creates a new ResizeOperation.
	@param d the Device to resize a Partition on
	@param p the Partition to resize
	@param newfirst the new first sector of the Partition
	@param newlast the new last sector of the Partition
*/
ResizeOperation::ResizeOperation(Device& d, Partition& p, qint64 newfirst, qint64 newlast) :
	Operation(),
	m_TargetDevice(d),
	m_Partition(p),
	m_OrigFirstSector(partition().firstSector()),
	m_OrigLastSector(partition().lastSector()),
	m_NewFirstSector(newfirst),
	m_NewLastSector(newlast),
	m_CheckOriginalJob(new CheckFileSystemJob(partition())),
	m_MoveExtendedJob(NULL),
	m_ShrinkResizeJob(NULL),
	m_ShrinkSetGeomJob(NULL),
	m_MoveSetGeomJob(NULL),
	m_MoveFileSystemJob(NULL),
	m_GrowResizeJob(NULL),
	m_GrowSetGeomJob(NULL),
	m_CheckResizedJob(NULL)
{
	addJob(checkOriginalJob());

	if (partition().roles().has(PartitionRole::Extended))
	{
		m_MoveExtendedJob = new SetPartGeometryJob(targetDevice(), partition(), newFirstSector(), newLength());
		addJob(moveExtendedJob());
	}
	else
	{
		if (resizeAction() & Shrink)
		{
			m_ShrinkResizeJob = new ResizeFileSystemJob(targetDevice(), partition(), newLength());
			m_ShrinkSetGeomJob = new SetPartGeometryJob(targetDevice(), partition(), partition().firstSector(), newLength());

			addJob(shrinkResizeJob());
			addJob(shrinkSetGeomJob());
		}

		if ((resizeAction() & MoveLeft) || (resizeAction() & MoveRight))
		{
			// At this point, we need to set the partition's length to either the resized length, if it has already been
			// shrunk, or to the original length (it may or may not then later be grown, we don't care here)
			const qint64 currentLength = (resizeAction() & Shrink) ? newLength() : partition().length();

			m_MoveSetGeomJob = new SetPartGeometryJob(targetDevice(), partition(), newFirstSector(), currentLength);
			m_MoveFileSystemJob = new MoveFileSystemJob(targetDevice(), partition(), newFirstSector());

			addJob(moveSetGeomJob());
			addJob(moveFileSystemJob());
		}

		if (resizeAction() & Grow)
		{
			m_GrowSetGeomJob = new SetPartGeometryJob(targetDevice(), partition(), newFirstSector(), newLength());
			m_GrowResizeJob = new ResizeFileSystemJob(targetDevice(), partition(), newLength());

			addJob(growSetGeomJob());
			addJob(growResizeJob());
		}

		m_CheckResizedJob = new CheckFileSystemJob(partition());

		addJob(checkResizedJob());
	}
}

bool ResizeOperation::targets(const Device& d) const
{
	return d == targetDevice();
}

bool ResizeOperation::targets(const Partition& p) const
{
	return p == partition();
}

void ResizeOperation::preview()
{
	// If the operation has already been executed, the partition will of course have newFirstSector and
	// newLastSector as first and last sector. But to remove it from its original position, we need to
	// temporarily set these values back to where they were before the operation was executed.
	if (partition().firstSector() == newFirstSector() && partition().lastSector() == newLastSector())
	{
		partition().setFirstSector(origFirstSector());
		partition().setLastSector(origLastSector());
	}

	removePreviewPartition(targetDevice(), partition());

	partition().setFirstSector(newFirstSector());
	partition().setLastSector(newLastSector());

	insertPreviewPartition(targetDevice(), partition());
}

void ResizeOperation::undo()
{
	removePreviewPartition(targetDevice(), partition());
	partition().setFirstSector(origFirstSector());
	partition().setLastSector(origLastSector());
	insertPreviewPartition(targetDevice(), partition());
}

bool ResizeOperation::execute(Report& parent)
{
	bool rval = false;

	Report* report = parent.newChild(description());

	if ((rval = checkOriginalJob()->run(*report)))
	{
		// Extended partitions are a special case: They don't have any file systems and so there's no
		// need to move, shrink or grow their contents before setting the new geometry. In fact, trying
		// to first shrink THEN move would not work for an extended partition that has children, because
		// they might temporarily be outside the extended partition and the backend would not let us do that.
		if (moveExtendedJob())
		{
			if (!(rval = moveExtendedJob()->run(*report)))
				report->line() << xi18nc("@info/plain", "Moving extended partition <filename>%1</filename> failed.", partition().deviceNode());
		}
		else
		{
			// We run all three methods. Any of them returns true if it has nothing to do.
			rval = shrink(*report) && move(*report) && grow(*report);

			if (rval)
			{
				if (!(rval = checkResizedJob()->run(*report)))
					report->line() << xi18nc("@info/plain", "Checking partition <filename>%1</filename> after resize/move failed.", partition().deviceNode());
			}
			else
				report->line() << xi18nc("@info/plain", "Resizing/moving partition <filename>%1</filename> failed.", partition().deviceNode());
		}
	}
	else
		report->line() << xi18nc("@info/plain", "Checking partition <filename>%1</filename> before resize/move failed.", partition().deviceNode());

	setStatus(rval ? StatusFinishedSuccess : StatusError);

	report->setStatus(i18nc("@info/plain status (success, error, warning...) of operation", "%1: %2", description(), statusText()));

	return rval;
}

QString ResizeOperation::description() const
{
	// There are eight possible things a resize operation might do:
	// 1) Move a partition to the left (closer to the start of the disk)
	// 2) Move a partition to the right (closer to the end of the disk)
	// 3) Grow a partition
	// 4) Shrink a partition
	// 5) Move a partition to the left and grow it
	// 6) Move a partition to the right and grow it
	// 7) Move a partition to the left and shrink it
	// 8) Move a partition to the right and shrink it
	// Each of these needs a different description. And for reasons of i18n, we cannot
	// just concatenate strings together...

	const QString moveDelta = Capacity::formatByteSize(qAbs(newFirstSector() - origFirstSector()) * targetDevice().logicalSectorSize());

	const QString origCapacity = Capacity::formatByteSize(origLength() * targetDevice().logicalSectorSize());
	const QString newCapacity = Capacity::formatByteSize(newLength() * targetDevice().logicalSectorSize());

	switch(resizeAction())
	{
		case MoveLeft:
			return xi18nc("@info/plain describe resize/move action", "Move partition <filename>%1</filename> to the left by %2", partition().deviceNode(), moveDelta);

		case MoveRight:
			return xi18nc("@info/plain describe resize/move action", "Move partition <filename>%1</filename> to the right by %2", partition().deviceNode(), moveDelta);

		case Grow:
			return xi18nc("@info/plain describe resize/move action", "Grow partition <filename>%1</filename> from %2 to %3", partition().deviceNode(), origCapacity, newCapacity);

		case Shrink:
			return xi18nc("@info/plain describe resize/move action", "Shrink partition <filename>%1</filename> from %2 to %3", partition().deviceNode(), origCapacity, newCapacity);

		case MoveLeftGrow:
			return xi18nc("@info/plain describe resize/move action", "Move partition <filename>%1</filename> to the left by %2 and grow it from %3 to %4", partition().deviceNode(), moveDelta, origCapacity, newCapacity);

		case MoveRightGrow:
			return xi18nc("@info/plain describe resize/move action", "Move partition <filename>%1</filename> to the right by %2 and grow it from %3 to %4", partition().deviceNode(), moveDelta, origCapacity, newCapacity);

		case MoveLeftShrink:
			return xi18nc("@info/plain describe resize/move action", "Move partition <filename>%1</filename> to the left by %2 and shrink it from %3 to %4", partition().deviceNode(), moveDelta, origCapacity, newCapacity);

		case MoveRightShrink:
			return xi18nc("@info/plain describe resize/move action", "Move partition <filename>%1</filename> to the right by %2 and shrink it from %3 to %4", partition().deviceNode(), moveDelta, origCapacity, newCapacity);

		default:
			qWarning() << "Could not determine what to do with partition " << partition().deviceNode() << ".";
			break;
	}

	return i18nc("@info/plain describe resize/move action", "Unknown resize/move action.");
}

ResizeOperation::ResizeAction ResizeOperation::resizeAction() const
{
	ResizeAction action = None;

	// Grow?
	if (newLength() > origLength())
		action = Grow;

	// Shrink?
	if (newLength() < origLength())
		action = Shrink;

	// Move to the right?
	if (newFirstSector() > origFirstSector())
		action = static_cast<ResizeAction>(action | MoveRight);

	// Move to the left?
	if (newFirstSector() < origFirstSector())
		action = static_cast<ResizeAction>(action | MoveLeft);

	return action;
}

bool ResizeOperation::shrink(Report& report)
{
	if (shrinkResizeJob() && !shrinkResizeJob()->run(report))
	{
		report.line() << xi18nc("@info/plain", "Resize/move failed: Could not resize file system to shrink partition <filename>%1</filename>.", partition().deviceNode());
		return false;
	}

	if (shrinkSetGeomJob() && !shrinkSetGeomJob()->run(report))
	{
		report.line() << xi18nc("@info/plain", "Resize/move failed: Could not shrink partition <filename>%1</filename>.", partition().deviceNode());
		return false;

		/** @todo if this fails, no one undoes the shrinking of the file system above, because we
		rely upon there being a maximize job at the end, but that's no longer the case. */
	}

	return true;
}

bool ResizeOperation::move(Report& report)
{
	// We must make sure not to overwrite the partition's metadata if it's a logical partition
	// and we're moving to the left. The easiest way to achieve this is to move the
	// partition itself first (it's the backend's responsibility to then move the metadata) and
	// only afterwards copy the filesystem. Disadvantage: We need to move the partition
	// back to its original position if copyBlocks fails.
	const qint64 oldStart = partition().firstSector();
	if (moveSetGeomJob() && !moveSetGeomJob()->run(report))
	{
		report.line() << xi18nc("@info/plain", "Moving partition <filename>%1</filename> failed.", partition().deviceNode());
		return false;
	}

	if (moveFileSystemJob() && !moveFileSystemJob()->run(report))
	{
		report.line() << xi18nc("@info/plain", "Moving the filesystem for partition <filename>%1</filename> failed. Rolling back.", partition().deviceNode());

		// see above: We now have to move back the partition itself.
		if (!SetPartGeometryJob(targetDevice(), partition(), oldStart, partition().length()).run(report))
			report.line() << xi18nc("@info/plain", "Moving back partition <filename>%1</filename> to its original position failed.", partition().deviceNode());

		return false;
	}

	return true;
}

bool ResizeOperation::grow(Report& report)
{
	const qint64 oldLength = partition().length();

	if (growSetGeomJob() && !growSetGeomJob()->run(report))
	{
		report.line() << xi18nc("@info/plain", "Resize/move failed: Could not grow partition <filename>%1</filename>.", partition().deviceNode());
		return false;
	}

	if (growResizeJob() && !growResizeJob()->run(report))
	{
		report.line() << xi18nc("@info/plain", "Resize/move failed: Could not resize the file system on partition <filename>%1</filename>", partition().deviceNode());

		if (!SetPartGeometryJob(targetDevice(), partition(), partition().firstSector(), oldLength).run(report))
			report.line() << xi18nc("@info/plain", "Could not restore old partition size for partition <filename>%1</filename>.", partition().deviceNode());

		return false;
	}

	return true;
}

/** Can a Partition be grown, i.e. increased in size?
	@param p the Partition in question, may be NULL.
	@return true if @p p can be grown.
 */
bool ResizeOperation::canGrow(const Partition* p)
{
	if (p == NULL)
		return false;

	// we can always grow, shrink or move a partition not yet written to disk
	if (p->state() == Partition::StateNew)
		return true;

	if (p->isMounted())
		return false;

	return p->fileSystem().supportGrow() != FileSystem::cmdSupportNone;
}

/** Can a Partition be shrunk, i.e. decreased in size?
	@param p the Partition in question, may be NULL.
	@return true if @p p can be shrunk.
 */
bool ResizeOperation::canShrink(const Partition* p)
{
	if (p == NULL)
		return false;

	// we can always grow, shrink or move a partition not yet written to disk
	if (p->state() == Partition::StateNew)
		return true;

	if (p->state() == Partition::StateCopy)
		return false;

	if (p->isMounted())
		return false;

	return p->fileSystem().supportShrink() != FileSystem::cmdSupportNone;
}

/** Can a Partition be moved?
	@param p the Partition in question, may be NULL.
	@return true if @p p can be moved.
 */
bool ResizeOperation::canMove(const Partition* p)
{
	if (p == NULL)
		return false;

	// we can always grow, shrink or move a partition not yet written to disk
	if (p->state() == Partition::StateNew)
		return true;

	if (p->isMounted())
		return false;

	// no moving of extended partitions if they have logicals
	if (p->roles().has(PartitionRole::Extended) && p->hasChildren())
		return false;

	return p->fileSystem().supportMove() != FileSystem::cmdSupportNone;
}
