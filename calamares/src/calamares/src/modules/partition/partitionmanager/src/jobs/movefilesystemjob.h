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

#if !defined(MOVEFILESYSTEMJOB__H)

#define MOVEFILESYSTEMJOB__H

#include "jobs/job.h"

class Partition;
class Device;
class Report;

class QString;

/** Move a FileSystem.

	Moves a FileSystem on a given Device and Partition to a new start sector.

	@author Volker Lanz <vl@fidra.de>
*/
class MoveFileSystemJob : public Job
{
	public:
		MoveFileSystemJob(Device& d, Partition& p, qint64 newstart);

	public:
		virtual bool run(Report& parent);
		virtual qint32 numSteps() const;
		virtual QString description() const;

	protected:
		Partition& partition() { return m_Partition; }
		const Partition& partition() const { return m_Partition; }

		Device& device() { return m_Device; }
		const Device& device() const { return m_Device; }

		qint64 newStart() const { return m_NewStart; }

	private:
		Device& m_Device;
		Partition& m_Partition;
		qint64 m_NewStart;
};

#endif
