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

#if !defined(COPYOPERATION__H)

#define COPYOPERATION__H

#include "ops/operation.h"

#include <QString>

class Partition;
class OperationStack;
class Device;
class Report;

class CreatePartitionJob;
class CheckFileSystemJob;
class CopyFileSystemJob;
class ResizeFileSystemJob;

/** Copy a Partition.

	Copies a Partition from a given source Device to a Partition on a given target Device and handles overwriting
	the target Partition in case that is required.

	@author Volker Lanz <vl@fidra.de>
*/
class CopyOperation : public Operation
{
	friend class OperationStack;

	Q_OBJECT
	Q_DISABLE_COPY(CopyOperation)

	public:
		CopyOperation(Device& targetdevice, Partition* copiedpartition, Device& sourcedevice, Partition* sourcepartition);
		~CopyOperation();

	public:
		QString iconName() const { return QStringLiteral("edit-copy"); }
		QString description() const { return m_Description; }

		bool execute(Report& parent);
		void preview();
		void undo();

		virtual bool targets(const Device& d) const;
		virtual bool targets(const Partition& p) const;

		static bool canCopy(const Partition* p);
		static bool canPaste(const Partition* p, const Partition* source);

		static Partition* createCopy(const Partition& target, const Partition& source);

	protected:
		Partition& copiedPartition() { return *m_CopiedPartition; }
		const Partition& copiedPartition() const { return *m_CopiedPartition; }

		Device& targetDevice() { return m_TargetDevice; }
		const Device& targetDevice() const { return m_TargetDevice; }

		Device& sourceDevice() { return m_SourceDevice; }
		const Device& sourceDevice() const { return m_SourceDevice; }

		Partition& sourcePartition() { return *m_SourcePartition; }
		const Partition& sourcePartition() const { return *m_SourcePartition; }

		Partition* overwrittenPartition() { return m_OverwrittenPartition; }
		const Partition* overwrittenPartition() const { return m_OverwrittenPartition; }

		void setOverwrittenPartition(Partition* p);
		void setSourcePartition(Partition* p) { m_SourcePartition = p; }

		void cleanupOverwrittenPartition();
		bool mustDeleteOverwritten() const { return m_MustDeleteOverwritten; }

		CheckFileSystemJob* checkSourceJob() { return m_CheckSourceJob; }
		CreatePartitionJob* createPartitionJob() {return m_CreatePartitionJob; }
		CopyFileSystemJob* copyFSJob() { return m_CopyFSJob; }
		CheckFileSystemJob* checkTargetJob() { return m_CheckTargetJob; }
		ResizeFileSystemJob* maximizeJob() { return m_MaximizeJob; }

		QString updateDescription() const;

	private:
		Device& m_TargetDevice;
		Partition* m_CopiedPartition;
		Device& m_SourceDevice;
		Partition* m_SourcePartition;
		Partition* m_OverwrittenPartition;
		bool m_MustDeleteOverwritten;

		CheckFileSystemJob* m_CheckSourceJob;
		CreatePartitionJob* m_CreatePartitionJob;
		CopyFileSystemJob* m_CopyFSJob;
		CheckFileSystemJob* m_CheckTargetJob;
		ResizeFileSystemJob* m_MaximizeJob;

		QString m_Description;
};

#endif
