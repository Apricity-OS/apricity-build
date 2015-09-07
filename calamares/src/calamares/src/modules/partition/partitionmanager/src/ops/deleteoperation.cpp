/***************************************************************************
 *   Copyright (C) 2008, 2010 by Volker Lanz <vl@fidra.de>                 *
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

#include "ops/deleteoperation.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/partitiontable.h"

#include "jobs/deletepartitionjob.h"
#include "jobs/deletefilesystemjob.h"
#include "jobs/shredfilesystemjob.h"

#include "util/capacity.h"

#include <QString>

#include <KLocalizedString>

/** Creates a new DeleteOperation
	@param d the Device to delete a Partition on
	@param p pointer to the Partition to delete. May not be NULL
*/
DeleteOperation::DeleteOperation(Device& d, Partition* p, bool secure) :
	Operation(),
	m_TargetDevice(d),
	m_DeletedPartition(p),
	m_Secure(secure),
	m_DeleteFileSystemJob(isSecure()
		? static_cast<Job*>(new ShredFileSystemJob(targetDevice(), deletedPartition()))
		: static_cast<Job*>(new DeleteFileSystemJob(targetDevice(), deletedPartition()))),
	m_DeletePartitionJob(new DeletePartitionJob(targetDevice(), deletedPartition()))
{
	addJob(deleteFileSystemJob());
	addJob(deletePartitionJob());
}

DeleteOperation::~DeleteOperation()
{
	if (status() != StatusPending && status() != StatusNone) // don't delete the partition if we're being merged or undone
		delete m_DeletedPartition;
}

bool DeleteOperation::targets(const Device& d) const
{
	return d == targetDevice();
}

bool DeleteOperation::targets(const Partition& p) const
{
	return p == deletedPartition();
}

void DeleteOperation::preview()
{
	removePreviewPartition(targetDevice(), deletedPartition());
	checkAdjustLogicalNumbers(deletedPartition(), false);
}

void DeleteOperation::undo()
{
	checkAdjustLogicalNumbers(deletedPartition(), true);
	insertPreviewPartition(targetDevice(), deletedPartition());
}

QString DeleteOperation::description() const
{
	if (isSecure())
		return xi18nc("@info/plain", "Shred partition <filename>%1</filename> (%2, %3)", deletedPartition().deviceNode(), Capacity::formatByteSize(deletedPartition().capacity()), deletedPartition().fileSystem().name());
	else
		return xi18nc("@info/plain", "Delete partition <filename>%1</filename> (%2, %3)", deletedPartition().deviceNode(), Capacity::formatByteSize(deletedPartition().capacity()), deletedPartition().fileSystem().name());
}

void DeleteOperation::checkAdjustLogicalNumbers(Partition& p, bool undo)
{
	// If the deleted partition is a logical one, we need to adjust the numbers of the
	// other logical partitions in the extended one, if there are any, because the OS
	// will do that, too: Logicals must be numbered without gaps, i.e., a numbering like
	// sda5, sda6, sda8 (after sda7 is deleted) will become sda5, sda6, sda7
	Partition* parentPartition = dynamic_cast<Partition*>(p.parent());
	if (parentPartition && parentPartition->roles().has(PartitionRole::Extended))
		parentPartition->adjustLogicalNumbers(undo ? -1 : p.number(), undo ? p.number() : -1);
}

/** Can a Partition be deleted?
	@param p the Partition in question, may be NULL.
	@return true if @p p can be deleted.
*/
bool DeleteOperation::canDelete(const Partition* p)
{
	if (p == NULL)
		return false;

	if (p->isMounted())
		return false;

	if (p->roles().has(PartitionRole::Unallocated))
		return false;

	if (p->roles().has(PartitionRole::Extended))
		return p->children().size() == 1 && p->children()[0]->roles().has(PartitionRole::Unallocated);

	return true;
}
