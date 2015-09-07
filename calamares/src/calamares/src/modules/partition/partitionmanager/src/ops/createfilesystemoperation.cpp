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

#include "ops/createfilesystemoperation.h"

#include "core/partition.h"
#include "core/device.h"

#include "jobs/deletefilesystemjob.h"
#include "jobs/createfilesystemjob.h"
#include "jobs/checkfilesystemjob.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"

#include <QString>

#include <KLocalizedString>

/** Creates a new CreateFileSystemOperation.
	@param d the Device to create the new FileSystem on
	@param p the Partition to create the new FileSystem in
	@param newType the type of the new FileSystem
*/
CreateFileSystemOperation::CreateFileSystemOperation(Device& d, Partition& p, FileSystem::Type newType) :
	Operation(),
	m_TargetDevice(d),
	m_Partition(p),
	m_NewFileSystem(FileSystemFactory::cloneWithNewType(newType, partition().fileSystem())),
	m_OldFileSystem(&p.fileSystem()),
	m_DeleteJob(new DeleteFileSystemJob(targetDevice(), partition())),
	m_CreateJob(new CreateFileSystemJob(targetDevice(), partition())),
	m_CheckJob(new CheckFileSystemJob(partition()))
{
	// We never know anything about the number of used sectors on a new file system.
	newFileSystem()->setSectorsUsed(-1);

	addJob(deleteJob());
	addJob(createJob());
	addJob(checkJob());
}

CreateFileSystemOperation::~CreateFileSystemOperation()
{
	if (&partition().fileSystem() == newFileSystem())
		delete oldFileSystem();
	else
		delete newFileSystem();
}

bool CreateFileSystemOperation::targets(const Device& d) const
{
	return d == targetDevice();
}

bool CreateFileSystemOperation::targets(const Partition& p) const
{
	return p == partition();
}

void CreateFileSystemOperation::preview()
{
	partition().setFileSystem(newFileSystem());
}

void CreateFileSystemOperation::undo()
{
	partition().setFileSystem(oldFileSystem());
}

bool CreateFileSystemOperation::execute(Report& parent)
{
	preview();

	return Operation::execute(parent);
}

QString CreateFileSystemOperation::description() const
{
	return xi18nc("@info/plain", "Create filesystem %1 on partition <filename>%2</filename>", newFileSystem()->name(), partition().deviceNode());
}
