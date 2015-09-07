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

#if !defined(COPYFILESYSTEMJOB__H)

#define COPYFILESYSTEMJOB__H

#include "jobs/job.h"

#include <qglobal.h>

class Partition;
class Device;
class Report;

class QString;

/** Copy a FileSystem.

	Copy a FileSystem on a given Partition and Device to another Partition on a (possibly other) Device.

	@author Volker Lanz <vl@fidra.de>
*/
class CopyFileSystemJob : public Job
{
	public:
		CopyFileSystemJob(Device& targetdevice, Partition& targetpartition, Device& sourcedevice, Partition& sourcepartition);

	public:
		virtual bool run(Report& parent);
		virtual qint32 numSteps() const;
		virtual QString description() const;

	protected:
		Partition& targetPartition() { return m_TargetPartition; }
		const Partition& targetPartition() const { return m_TargetPartition; }
		
		Device& targetDevice() { return m_TargetDevice; }
		const Device& targetDevice() const { return m_TargetDevice; }
		
		Partition& sourcePartition() { return m_SourcePartition; }
		const Partition& sourcePartition() const { return m_SourcePartition; }
		
		Device& sourceDevice() { return m_SourceDevice; }
		const Device& sourceDevice() const { return m_SourceDevice; }

	private:
		Device& m_TargetDevice;
		Partition& m_TargetPartition;
		Device& m_SourceDevice;
		Partition& m_SourcePartition;
};

#endif
