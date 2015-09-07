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

#if !defined(CREATEPARTITIONTABLEJOB__H)

#define CREATEPARTITIONTABLEJOB__H

#include "jobs/job.h"

class Device;
class Report;

class QString;

/** Create a PartitionTable.
	@author Volker Lanz <vl@fidra.de>
*/
class CreatePartitionTableJob : public Job
{
	public:
		CreatePartitionTableJob(Device& d);

	public:
		virtual bool run(Report& parent);
		virtual QString description() const;

	protected:
		Device& device() { return m_Device; }
		const Device& device() const { return m_Device; }

	private:
		Device& m_Device;
};

#endif
