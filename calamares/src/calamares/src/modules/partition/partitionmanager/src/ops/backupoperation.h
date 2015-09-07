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

#if !defined(BACKUPOPERATION__H)

#define BACKUPOPERATION__H

#include "ops/operation.h"

#include <QString>

class Partition;
class Device;
class BackupFileSystemJob;

/** Back up a FileSystem.
	@author Volker Lanz <vl@fidra.de>
*/
class BackupOperation : public Operation
{
	Q_OBJECT
	Q_DISABLE_COPY(BackupOperation)

	public:
		BackupOperation(Device& targetDevice, Partition& backupPartition, const QString& filename);

	public:
		QString iconName() const { return QStringLiteral("document-export"); }
		QString description() const;
		void preview() {}
		void undo() {}

		virtual bool targets(const Device&) const { return false; }
		virtual bool targets(const Partition&) const { return false; }

		static bool canBackup(const Partition* p);

	protected:
		Device& targetDevice() { return m_TargetDevice; }
		const Device& targetDevice() const { return m_TargetDevice; }

		Partition& backupPartition() { return m_BackupPartition; }
		const Partition& backupPartition() const { return m_BackupPartition; }

		const QString& fileName() const { return m_FileName; }

		BackupFileSystemJob* backupJob() { return m_BackupJob; }

	private:
		Device& m_TargetDevice;
		Partition& m_BackupPartition;
		const QString m_FileName;
		BackupFileSystemJob* m_BackupJob;
};

#endif
