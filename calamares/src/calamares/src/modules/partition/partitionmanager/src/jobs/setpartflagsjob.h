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

#if !defined(SETPARTFLAGSJOB__H)

#define SETPARTFLAGSJOB__H

#include "jobs/job.h"

#include "core/partitiontable.h"

class Device;
class Partition;
class Report;

class QString;

/** Set a Partition's flags.

	Set the Partition flags for a given Partition on a given Device.

	@author Volker Lanz <vl@fidra.de>
*/
class SetPartFlagsJob : public Job
{
	public:
		SetPartFlagsJob(Device& d, Partition& p, PartitionTable::Flags flags);

	public:
		virtual bool run(Report& parent);
		virtual qint32 numSteps() const;
		virtual QString description() const;

	protected:
		Device& device() { return m_Device; }
		const Device& device() const { return m_Device; }

		Partition& partition() { return m_Partition; }
		const Partition& partition() const { return m_Partition; }

		PartitionTable::Flags flags() const { return m_Flags; }

	private:
		Device& m_Device;
		Partition& m_Partition;
		PartitionTable::Flags m_Flags;
};

#endif
