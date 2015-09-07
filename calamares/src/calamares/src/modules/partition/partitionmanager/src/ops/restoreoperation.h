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

#if !defined(RESTOREOPERATION__H)

#define RESTOREOPERATION__H

#include "ops/operation.h"

#include <QString>

class Partition;
class Device;
class OperationStack;
class Report;
class PartitionNode;

class CreatePartitionJob;
class RestoreFileSystemJob;
class CheckFileSystemJob;
class ResizeFileSystemJob;

/** Restore a Partition.

	Restores the FileSystem from a file to the given Partition on the given Device, handling overwriting
	a previous Partition in case that is necessary.

	@author Volker Lanz <vl@fidra.de>
*/
class RestoreOperation : public Operation
{
	friend class OperationStack;

	Q_OBJECT
	Q_DISABLE_COPY(RestoreOperation)

	public:
		RestoreOperation(Device& d, Partition* p, const QString& filename);
		~RestoreOperation();

	public:
		QString iconName() const { return QStringLiteral("document-import"); }
		QString description() const;
		bool execute(Report& parent);
		void undo();

		void preview();

		virtual bool targets(const Device& d) const;
		virtual bool targets(const Partition& p) const;

		static bool canRestore(const Partition* p);
		static Partition* createRestorePartition(const Device& device, PartitionNode& parent, qint64 start, const QString& fileName);

	protected:
		Device& targetDevice() { return m_TargetDevice; }
		const Device& targetDevice() const { return m_TargetDevice; }

		Partition& restorePartition() { return *m_RestorePartition; }
		const Partition& restorePartition() const { return *m_RestorePartition; }

		const QString& fileName() const { return m_FileName; }

		Partition* overwrittenPartition() { return m_OverwrittenPartition; }
		const Partition* overwrittenPartition() const { return m_OverwrittenPartition; }
		void setOverwrittenPartition(Partition* p);

		void cleanupOverwrittenPartition();
		bool mustDeleteOverwritten() const { return m_MustDeleteOverwritten; }

		qint64 imageLength() const { return m_ImageLength; }

		CreatePartitionJob* createPartitionJob() { return m_CreatePartitionJob; }
		RestoreFileSystemJob* restoreJob() { return m_RestoreJob; }
		CheckFileSystemJob* checkTargetJob() { return m_CheckTargetJob; }
		ResizeFileSystemJob* maximizeJob() { return m_MaximizeJob; }

	private:
		Device& m_TargetDevice;
		Partition* m_RestorePartition;
		const QString m_FileName;
		Partition* m_OverwrittenPartition;
		bool m_MustDeleteOverwritten;
		qint64 m_ImageLength;
		CreatePartitionJob* m_CreatePartitionJob;
		RestoreFileSystemJob* m_RestoreJob;
		CheckFileSystemJob* m_CheckTargetJob;
		ResizeFileSystemJob* m_MaximizeJob;
};

#endif
