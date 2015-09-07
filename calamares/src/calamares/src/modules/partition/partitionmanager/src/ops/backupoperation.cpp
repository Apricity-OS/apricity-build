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

#include "ops/backupoperation.h"

#include "core/partition.h"
#include "core/device.h"

#include "jobs/backupfilesystemjob.h"

#include "util/capacity.h"

#include <QString>

#include <KLocalizedString>

/** Creates a new BackupOperation.
	@param d the Device where the FileSystem to back up is on
	@param p the Partition where the FileSystem to back up is in
	@param filename the name of the file to back up to
*/
BackupOperation::BackupOperation(Device& d, Partition& p, const QString& filename) :
	Operation(),
	m_TargetDevice(d),
	m_BackupPartition(p),
	m_FileName(filename),
	m_BackupJob(new BackupFileSystemJob(targetDevice(), backupPartition(), fileName()))
{
	addJob(backupJob());
}

QString BackupOperation::description() const
{
	return xi18nc("@info/plain", "Backup partition <filename>%1</filename> (%2, %3) to <filename>%4</filename>", backupPartition().deviceNode(), Capacity::formatByteSize(backupPartition().capacity()), backupPartition().fileSystem().name(), fileName());
}

/** Can the given Partition be backed up?
	@param p The Partition in question, may be NULL.
	@return true if @p p can be backed up.
*/
bool BackupOperation::canBackup(const Partition* p)
{
	if (p == NULL)
		return false;

	if (p->isMounted())
		return false;

	if (p->state() == Partition::StateNew || p->state() == Partition::StateCopy || p->state() == Partition::StateRestore)
		return false;

	return p->fileSystem().supportBackup() != FileSystem::cmdSupportNone;
}

