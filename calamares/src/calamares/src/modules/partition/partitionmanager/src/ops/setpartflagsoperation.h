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

#if !defined(SETPARTFLAGSOPERATION__H)

#define SETPARTFLAGSOPERATION__H

#include "ops/operation.h"

#include "core/partitiontable.h"

#include <QString>

class Device;
class OperationStack;
class Partition;

class SetPartFlagsJob;

/** Set Partition flags.

	Sets the Partition flags for the given Partition on the given Device.

	@author Volker Lanz <vl@fidra.de>
*/
class SetPartFlagsOperation : public Operation
{
	friend class OperationStack;

	Q_OBJECT
	Q_DISABLE_COPY(SetPartFlagsOperation)

	public:
		SetPartFlagsOperation(Device& d, Partition& p, const PartitionTable::Flags& flags);

	public:
		QString iconName() const { return QStringLiteral("flag-blue"); }
		QString description() const;
		void preview();
		void undo();

		virtual bool targets(const Device& d) const;
		virtual bool targets(const Partition& p) const;

	protected:
		Partition& flagPartition() { return m_FlagPartition; }
		const Partition& flagPartition() const { return m_FlagPartition; }

		Device& targetDevice() { return m_TargetDevice; }
		const Device& targetDevice() const { return m_TargetDevice; }

		const PartitionTable::Flags& oldFlags() const { return m_OldFlags; }
		const PartitionTable::Flags& newFlags() const { return m_NewFlags; }

		void setOldFlags(PartitionTable::Flags f) { m_OldFlags = f; }

		SetPartFlagsJob* flagsJob() { return m_FlagsJob; }

	private:
		Device& m_TargetDevice;
		Partition& m_FlagPartition;
		PartitionTable::Flags m_OldFlags;
		PartitionTable::Flags m_NewFlags;
		SetPartFlagsJob* m_FlagsJob;
};

#endif
