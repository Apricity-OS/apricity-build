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

#include "ops/setpartflagsoperation.h"

#include "core/partition.h"
#include "core/partitionnode.h"
#include "core/partitiontable.h"
#include "core/device.h"

#include "jobs/setpartflagsjob.h"

#include "fs/filesystem.h"

#include <QString>

#include <KLocalizedString>

/** Creates a new SetPartFlagsOperation.
	@param d the Device on which the Partition to set flags for is
	@param p the Partition to set new flags for
	@param flags the new flags to set
*/
SetPartFlagsOperation::SetPartFlagsOperation(Device& d, Partition& p, const PartitionTable::Flags& flags) :
	Operation(),
	m_TargetDevice(d),
	m_FlagPartition(p),
	m_OldFlags(flagPartition().activeFlags()),
	m_NewFlags(flags),
	m_FlagsJob(new SetPartFlagsJob(targetDevice(), flagPartition(), newFlags()))
{
	addJob(flagsJob());
}

bool SetPartFlagsOperation::targets(const Device& d) const
{
	return d == targetDevice();
}

bool SetPartFlagsOperation::targets(const Partition& p) const
{
	return p == flagPartition();
}

void SetPartFlagsOperation::preview()
{
	flagPartition().setFlags(newFlags());
}

void SetPartFlagsOperation::undo()
{
	flagPartition().setFlags(oldFlags());
}

QString SetPartFlagsOperation::description() const
{
	if (PartitionTable::flagNames(newFlags()).size() == 0)
		return xi18nc("@info/plain", "Clear flags for partition <filename>%1</filename>", flagPartition().deviceNode());

	return xi18nc("@info/plain", "Set flags for partition <filename>%1</filename> to \"%2\"", flagPartition().deviceNode(), PartitionTable::flagNames(newFlags()).join(QStringLiteral(",")));
}
