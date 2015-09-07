/***************************************************************************
 *   Copyright (C) 2010 by Volker Lanz <vl@fidra.de                        *
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

#include "plugins/libparted/libpartedpartition.h"
#include "plugins/libparted/libpartedbackend.h"

#include "util/report.h"

#include <KLocalizedString>

LibPartedPartition::LibPartedPartition(PedPartition* ped_partition) :
	CoreBackendPartition(),
	m_PedPartition(ped_partition)
{
}

bool LibPartedPartition::setFlag(Report& report, PartitionTable::Flag partitionManagerFlag, bool state)
{
	Q_ASSERT(pedPartition() != NULL);

	const PedPartitionFlag f = LibPartedBackend::getPedFlag(partitionManagerFlag);

	// ignore flags that don't exist for this partition
	if (!ped_partition_is_flag_available(pedPartition(), f))
	{
		report.line() << i18nc("@info/plain", "The flag \"%1\" is not available on the partition's partition table.", PartitionTable::flagName(partitionManagerFlag));
		return true;
	}

	// Workaround: libparted claims the hidden flag is available for extended partitions, but
	// throws an error when we try to set or clear it. So skip this combination.
	if (pedPartition()->type == PED_PARTITION_EXTENDED && partitionManagerFlag == PartitionTable::FlagHidden)
		return true;

	if (!ped_partition_set_flag(pedPartition(), f, state ? 1 : 0))
		return false;

	return true;
}

