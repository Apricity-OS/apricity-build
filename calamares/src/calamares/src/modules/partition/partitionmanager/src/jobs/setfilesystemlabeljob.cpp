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

#include "jobs/setfilesystemlabeljob.h"

#include "core/partition.h"

#include "fs/filesystem.h"

#include "util/report.h"

#include <KLocalizedString>

/** Creates a new SetFileSystemLabelJob
	@param p the Partition the FileSystem whose label is to be set is on
	@param newlabel the new label
*/
SetFileSystemLabelJob::SetFileSystemLabelJob(Partition& p, const QString& newlabel) :
	Job(),
	m_Partition(p),
	m_Label(newlabel)
{
}

bool SetFileSystemLabelJob::run(Report& parent)
{
	bool rval = true;

	Report* report = jobStarted(parent);

	// If there's no support for file system label setting for this file system,
	// just ignore the request and say all is well. This helps in operations because
	// we don't have to check for support to avoid having a failed job.
	if (partition().fileSystem().supportSetLabel() == FileSystem::cmdSupportNone)
		report->line() << xi18nc("@info/plain", "File system on partition <filename>%1</filename> does not support setting labels. Job ignored.", partition().deviceNode());
	else if (partition().fileSystem().supportSetLabel() == FileSystem::cmdSupportFileSystem)
	{
		rval = partition().fileSystem().writeLabel(*report, partition().deviceNode(), label());

		if (rval)
			partition().fileSystem().setLabel(label());
	}

	jobFinished(*report, rval);

	return rval;
}

QString SetFileSystemLabelJob::description() const
{
	return xi18nc("@info/plain", "Set the file system label on partition <filename>%1</filename> to \"%2\"", partition().deviceNode(), label());
}
