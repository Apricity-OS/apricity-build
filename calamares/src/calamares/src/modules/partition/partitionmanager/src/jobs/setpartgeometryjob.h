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

#if !defined(SETPARTGEOMETRYJOB__H)

#define SETPARTGEOMETRYJOB__H

#include "jobs/job.h"

#include <qglobal.h>

class Partition;
class Device;
class Report;

class QString;

/** Set a Partition's geometry.

	Sets the geometry for a given Partition on a given Device to a new start sector and/or a new
	length. This does not move the FileSystem, it only updates the partition table entry for the
	Partition and is usually run together with MoveFileSystemJob or ResizeFileSystemJob for that reason.

	@author Volker Lanz <vl@fidra.de>
*/
class SetPartGeometryJob : public Job
{
	public:
		SetPartGeometryJob(Device& d, Partition& p, qint64 newstart, qint64 newlength);

	public:
		virtual bool run(Report& parent);
		virtual QString description() const;

	protected:
		Partition& partition() { return m_Partition; }
		const Partition& partition() const { return m_Partition; }

		Device& device() { return m_Device; }
		const Device& device() const { return m_Device; }

		qint64 newStart() const { return m_NewStart; }
		qint64 newLength() const { return m_NewLength; }

	private:
		Device& m_Device;
		Partition& m_Partition;
		qint64 m_NewStart;
		qint64 m_NewLength;
};

#endif
