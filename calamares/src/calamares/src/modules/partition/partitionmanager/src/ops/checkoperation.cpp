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

#include "ops/checkoperation.h"

#include "core/partition.h"
#include "core/device.h"

#include "jobs/checkfilesystemjob.h"
#include "jobs/resizefilesystemjob.h"

#include "util/capacity.h"

#include <QString>

#include <KLocalizedString>

/** Creates a new CheckOperation.
	@param d the Device where the Partition to check is on.
	@param p the Partition to check
*/
CheckOperation::CheckOperation(Device& d, Partition& p) :
	Operation(),
	m_TargetDevice(d),
	m_CheckedPartition(p),
	m_CheckJob(new CheckFileSystemJob(checkedPartition())),
	m_MaximizeJob(new ResizeFileSystemJob(targetDevice(), checkedPartition()))
{
	addJob(checkJob());
	addJob(maximizeJob());
}

bool CheckOperation::targets(const Device& d) const
{
	return d == targetDevice();
}

bool CheckOperation::targets(const Partition& p) const
{
	return p == checkedPartition();
}

QString CheckOperation::description() const
{
	return xi18nc("@info/plain", "Check and repair partition <filename>%1</filename> (%2, %3)", checkedPartition().deviceNode(), Capacity::formatByteSize(checkedPartition().capacity()), checkedPartition().fileSystem().name());
}

/** Can a Partition be checked?
	@param p the Partition in question, may be NULL.
	@return true if @p p can be checked.
*/
bool CheckOperation::canCheck(const Partition* p)
{
	if (p == NULL)
		return false;

	if (p->isMounted())
		return false;

	return p->fileSystem().supportCheck() != FileSystem::cmdSupportNone;
}

