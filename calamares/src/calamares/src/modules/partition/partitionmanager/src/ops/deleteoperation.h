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

#if !defined(DELETEOPERATION__H)

#define DELETEOPERATION__H

#include "ops/operation.h"

#include <QString>

class Device;
class OperationStack;
class Partition;

class Job;
class DeletePartitionJob;

/** Delete a Partition.
	@author Volker Lanz <vl@fidra.de>
*/
class DeleteOperation : public Operation
{
	friend class OperationStack;

	Q_OBJECT
	Q_DISABLE_COPY(DeleteOperation)

	public:
		DeleteOperation(Device& d, Partition* p, bool secure = false);
		~DeleteOperation();

	public:
		QString iconName() const { return isSecure() ? QStringLiteral("edit-delete-shred") : QStringLiteral("edit-delete"); }
		QString description() const;
		void preview();
		void undo();
		bool isSecure() const { return m_Secure; }

		virtual bool targets(const Device& d) const;
		virtual bool targets(const Partition& p) const;

		static bool canDelete(const Partition* p);

	protected:
		Device& targetDevice() { return m_TargetDevice; }
		const Device& targetDevice() const { return m_TargetDevice; }

		Partition& deletedPartition() { return *m_DeletedPartition; }
		const Partition& deletedPartition() const { return *m_DeletedPartition; }

		void checkAdjustLogicalNumbers(Partition& p, bool undo);

		void setDeletedPartition(Partition* p) { m_DeletedPartition = p; }

		Job* deleteFileSystemJob() { return m_DeleteFileSystemJob; }
		DeletePartitionJob* deletePartitionJob() { return m_DeletePartitionJob; }

	private:
		Device& m_TargetDevice;
		Partition* m_DeletedPartition;
		bool m_Secure;
		Job* m_DeleteFileSystemJob;
		DeletePartitionJob* m_DeletePartitionJob;
};

#endif
