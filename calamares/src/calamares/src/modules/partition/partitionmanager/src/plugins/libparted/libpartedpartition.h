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

#if !defined(LIBPARTEDPARTITION__H)

#define LIBPARTEDPARTITION__H

#include "backend/corebackendpartition.h"

#include "core/partitiontable.h"

#include <parted/parted.h>

class Report;

class LibPartedPartition : public CoreBackendPartition
{
	Q_DISABLE_COPY(LibPartedPartition);

	public:
		LibPartedPartition(PedPartition* ped_partition);

	public:
		virtual bool setFlag(Report& report, PartitionTable::Flag flag, bool state);

	private:
		PedPartition* pedPartition() { return m_PedPartition; }

	private:
		PedPartition* m_PedPartition;
};


#endif
