/***************************************************************************
 *   Copyright (C) 2010,2011 by Volker Lanz <vl@fidra.de                   *
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

#if !defined(COREBACKENDPARTITIONTABLE__H)

#define COREBACKENDPARTITIONTABLE__H
#include "libpartitionmanager_export.h"

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

#include <qglobal.h>
#include <fatlabel/partition.h>

class CoreBackendPartition;
class Report;
class Partition;

/**
  * Interface class to represent a partition table in the backend.
  * @author Volker Lanz <vl@fidra.de>
  */
class LIBPARTITIONMANAGERPRIVATE_EXPORT CoreBackendPartitionTable
{
	public:
		virtual ~CoreBackendPartitionTable() {}

	public:
		/**
		  * Open the partition table
		  * @return true on success
		  */
		virtual bool open() = 0;

		/**
		  * Commit changes to the partition table to disk and to the OS.
		  * @param timeout timeout in seconds to wait for the commit to succeed
		  * @return true on success
		*/
		virtual bool commit(quint32 timeout = 10) = 0;

		/**
		  * @return pointer to the extended partition as a CoreBackendPartition or NULL if there is none
		  */
		virtual CoreBackendPartition* getExtendedPartition() = 0;

		/**
		  * @param sector sector the partition occupies
		  * @return the CoreBackendPartition to occupy the given sector or NULL if not found
		  */
		virtual CoreBackendPartition* getPartitionBySector(qint64 sector) = 0;

		/**
		  * Delete a partition.
		  * @param report the report to write information to
		  * @param partition the Partition to delete
		  * @return true on success
		  */
		virtual bool deletePartition(Report& report, const Partition& partition) = 0;

		/**
		  * Delete a file system on disk so it cannot be detected anymore.
		  * @param report the report to write information to
		  * @param partition the Partition for which to clobber the file system
		  * @return true on success
		  */
		virtual bool clobberFileSystem(Report& report, const Partition& partition) = 0;

		/**
		  * Resize a file system to a new length.
		  * @param report the report to write information to
		  * @param partition the partition the FileSystem to resize is on
		  * @param newLength the new length for the FileSystem in sectors
		  * @return true on success
		  */
		virtual bool resizeFileSystem(Report& report, const Partition& partition, qint64 newLength) = 0;

		/**
		  * Detect which FileSystem is present at a given start sector.
		  * @param report the report to write information to
		  * @param device the Device on which the FileSystem resides
		  * @param sector the sector where to look for a FileSystem
		  * @return the detected FileSystem::Type
		  */
		virtual FileSystem::Type detectFileSystemBySector(Report& report, const Device& device, qint64 sector) = 0;

		/**
		  * Create a new partition.
		  * @param report the report to write information to
		  * @param partition the new partition to create on disk
		  * @return the new number the OS sees the partition under (e.g. 7 for "/dev/sda7") or -1 on failure
		  */
		virtual QString createPartition(Report& report, const Partition& partition) = 0;

		/**
		  * Update the geometry for a partition in the partition table.
		  * @param report the report to write information to
		  * @param partition the partition to update the geometry for
		  * @param sector_start the new start sector for the partition
		  * @param sector_end the new last sector for the partition
		  * @return true on success
		  */
		virtual bool updateGeometry(Report& report, const Partition& partition, qint64 sector_start, qint64 sector_end) = 0;

		/**
		 * Set the system type (e.g. 83 for Linux) of a partition. The type to set is taken from
		 * the partition's file system.
		 * @param report the report to write information to
		 * @param partition the partition to set the system type for
		 * @return true on success
		 */
		virtual bool setPartitionSystemType(Report& report, const Partition& partition) = 0;
};

#endif
