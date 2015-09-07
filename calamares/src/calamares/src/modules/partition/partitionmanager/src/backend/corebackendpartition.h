/***************************************************************************
 *   Copyright (C) 2010 by Volker Lanz <vl@fidra.de                        *
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

#if !defined(COREBACKENDPARTITION__H)

#define COREBACKENDPARTITION__H

#include "util/libpartitionmanagerexport.h"

#include "core/partitiontable.h"

class Report;

/**
  * Represents a partition in the backend plugin.
  * @author Volker Lanz <vl@fidra.de>
  */
class LIBPARTITIONMANAGERPRIVATE_EXPORT CoreBackendPartition
{
	public:
		CoreBackendPartition();
		virtual ~CoreBackendPartition() {}

	public:
		/**
		  * Set a flag for the partition
		  * @param report the Report to write information to
		  * @param flag the flag to set
		  * @param state the state to set the flag to (i.e., on or off)
		  * @return true on success
		  */
		virtual bool setFlag(Report& report, PartitionTable::Flag flag, bool state) = 0;
};

#endif
