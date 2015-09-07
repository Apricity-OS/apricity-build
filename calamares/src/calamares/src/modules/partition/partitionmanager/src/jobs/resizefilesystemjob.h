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

#if !defined(RESIZEFILESYSTEMJOB__H)

#define RESIZEFILESYSTEMJOB__H

#include "jobs/job.h"

class Partition;
class Device;
class Report;

class QString;

/** Resize a FileSystem.

	Resizes a FileSystem on a given Device and Partition to a new length. If the new length is -1, the
	FileSystem is maximized to fill the entire Partition.

	@author Volker Lanz <vl@fidra.de>
*/
class ResizeFileSystemJob : public Job
{
	public:
		ResizeFileSystemJob(Device& d, Partition& p, qint64 newlength = -1);

	public:
		virtual bool run(Report& parent);
		virtual qint32 numSteps() const;
		virtual QString description() const;

	protected:
		bool resizeFileSystemBackend(Report& report);

		Partition& partition() { return m_Partition; }
		const Partition& partition() const { return m_Partition; }

		Device& device() { return m_Device; }
		const Device& device() const { return m_Device; }

		qint64 newLength() const { return m_NewLength; }

		bool isMaximizing() const { return m_Maximize; }

	private:
		Device& m_Device;
		Partition& m_Partition;
		bool m_Maximize;
		qint64 m_NewLength;
};

#endif
