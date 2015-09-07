/***************************************************************************
 *   Copyright (C) 2008,2010 by Volker Lanz <vl@fidra.de>                  *
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

#include "ops/createpartitiontableoperation.h"

#include "core/device.h"
#include "core/partitiontable.h"
#include "core/partition.h"

#include "jobs/createpartitiontablejob.h"

#include <QString>

#include <KLocalizedString>

/** Creates a new CreatePartitionTableOperation.
	@param d the Device to create the new PartitionTable on
	@param t the type for the new PartitionTable
*/
CreatePartitionTableOperation::CreatePartitionTableOperation(Device& d, PartitionTable::TableType t) :
	Operation(),
	m_TargetDevice(d),
	m_OldPartitionTable(targetDevice().partitionTable()),
	m_PartitionTable(new PartitionTable(t, PartitionTable::defaultFirstUsable(d, t), PartitionTable::defaultLastUsable(d, t))),
	m_CreatePartitionTableJob(new CreatePartitionTableJob(targetDevice()))
{
	addJob(createPartitionTableJob());
}

/** Creates a new CreatePartitionTableOperation.
	@param d the Device to create the new PartitionTable on
	@param ptable pointer to the new partition table object. the operation takes ownership.
*/
CreatePartitionTableOperation::CreatePartitionTableOperation(Device& d, PartitionTable* ptable) :
	Operation(),
	m_TargetDevice(d),
	m_OldPartitionTable(targetDevice().partitionTable()),
	m_PartitionTable(ptable),
	m_CreatePartitionTableJob(new CreatePartitionTableJob(targetDevice()))
{
	addJob(createPartitionTableJob());
}

CreatePartitionTableOperation::~CreatePartitionTableOperation()
{
	if (status() == StatusPending)
		delete m_PartitionTable;
}

bool CreatePartitionTableOperation::targets(const Device& d) const
{
	return d == targetDevice();
}

void CreatePartitionTableOperation::preview()
{
	targetDevice().setPartitionTable(partitionTable());
	targetDevice().partitionTable()->updateUnallocated(targetDevice());
}

void CreatePartitionTableOperation::undo()
{
	targetDevice().setPartitionTable(oldPartitionTable());

	if (targetDevice().partitionTable())
		targetDevice().partitionTable()->updateUnallocated(targetDevice());
}

bool CreatePartitionTableOperation::execute(Report& parent)
{
	targetDevice().setPartitionTable(partitionTable());
	return Operation::execute(parent);
}

/** Can a new partition table be created on a device?
	@param device pointer to the device, can be NULL
	@return true if a new partition table can be created on @p device
*/
bool CreatePartitionTableOperation::canCreate(const Device* device)
{
	return device != NULL && (device->partitionTable() == NULL || !device->partitionTable()->isChildMounted());
}

QString CreatePartitionTableOperation::description() const
{
	return xi18nc("@info/plain", "Create a new partition table (type: %1) on <filename>%2</filename>", partitionTable()->typeName(), targetDevice().deviceNode());
}
