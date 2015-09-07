/***************************************************************************
 *   Copyright (C) 2008,2011 by Volker Lanz <vl@fidra.de>                  *
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

#if !defined(CHECKOPERATION__H)

#define CHECKOPERATION__H

#include "ops/operation.h"

#include <QString>

class Partition;
class Device;
class CheckFileSystemJob;
class ResizeFileSystemJob;

/** Check a Partition.
	@author Volker Lanz <vl@fidra.de>
*/
class CheckOperation : public Operation
{
	friend class OperationStack;

	Q_OBJECT
	Q_DISABLE_COPY(CheckOperation)

	public:
		CheckOperation(Device& targetDevice, Partition& checkedPartition);

	public:
		QString iconName() const { return QStringLiteral("flag"); }
		QString description() const;
		void preview() {}
		void undo() {}

		virtual bool targets(const Device& d) const;
		virtual bool targets(const Partition& p) const;

		static bool canCheck(const Partition* p);

	protected:
		Device& targetDevice() { return m_TargetDevice; }
		const Device& targetDevice() const { return m_TargetDevice; }

		Partition& checkedPartition() { return m_CheckedPartition; }
		const Partition& checkedPartition() const { return m_CheckedPartition; }

		CheckFileSystemJob* checkJob() { return m_CheckJob; }
		ResizeFileSystemJob* maximizeJob() { return m_MaximizeJob; }

	private:
		Device& m_TargetDevice;
		Partition& m_CheckedPartition;
		CheckFileSystemJob* m_CheckJob;
		ResizeFileSystemJob* m_MaximizeJob;
};

#endif
