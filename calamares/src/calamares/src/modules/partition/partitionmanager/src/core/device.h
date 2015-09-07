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

#if !defined(DEVICE__H)

#define DEVICE__H

#include "util/libpartitionmanagerexport.h"

#include <QString>
#include <QObject>
#include <qglobal.h>

class PartitionTable;
class CreatePartitionTableOperation;
class CoreBackend;
class SmartStatus;

/** A device.

	Represents a device like /dev/sda.

	Devices are the outermost entity; they contain a PartitionTable that itself contains Partitions.

	@see PartitionTable, Partition
	@author Volker Lanz <vl@fidra.de>
*/
class LIBPARTITIONMANAGERPRIVATE_EXPORT Device : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(Device)

	friend class CreatePartitionTableOperation;
	friend class CoreBackend;

	public:
		Device(const QString& name, const QString& devicenode, qint32 heads, qint32 numSectors, qint32 cylinders, qint64 sectorSize, const QString& iconname = QString());
		~Device();

	public:
		bool operator==(const Device& other) const;
		bool operator!=(const Device& other) const;

		const QString& name() const { return m_Name; } /**< @return the Device's name, usually some manufacturer string */
		const QString& deviceNode() const { return m_DeviceNode; } /**< @return the Device's node, for example "/dev/sda" */
		PartitionTable* partitionTable() { return m_PartitionTable; } /**< @return the Device's PartitionTable */
		const PartitionTable* partitionTable() const { return m_PartitionTable; } /**< @return the Device's PartitionTable */
		qint32 heads() const { return m_Heads; } /**< @return the number of heads on the Device in CHS notation */
		qint32 cylinders() const { return m_Cylinders; } /**< @return the number of cylinders on the Device in CHS notation */
		qint32 sectorsPerTrack() const { return m_SectorsPerTrack; } /**< @return the number of sectors on the Device in CHS notation */
		qint32 physicalSectorSize() const { return m_PhysicalSectorSize; }  /**< @return the physical sector size the Device uses or -1 if unknown */
		qint32 logicalSectorSize() const { return m_LogicalSectorSize; } /**< @return the logical sector size the Device uses */
		qint64 totalSectors() const { return static_cast<qint64>(heads()) * cylinders() * sectorsPerTrack(); } /**< @return the total number of sectors on the device */
		qint64 capacity() const { return totalSectors() * logicalSectorSize(); } /**< @return the Device's capacity in bytes */
		qint64 cylinderSize() const { return static_cast<qint64>(heads()) * sectorsPerTrack(); } /**< @return the size of a cylinder on this Device in sectors */

		void setIconName(const QString& name) { m_IconName = name; }
		const QString& iconName() const { return m_IconName; } /**< @return suggested icon name for this Device */

		SmartStatus& smartStatus() { return *m_SmartStatus; }
		const SmartStatus& smartStatus() const { return *m_SmartStatus; }

		QString prettyName() const;

#ifndef CALAMARES
	protected:
#endif
		void setPartitionTable(PartitionTable* ptable) { m_PartitionTable = ptable; }

	private:
		QString m_Name;
		QString m_DeviceNode;
		PartitionTable* m_PartitionTable;
		qint32 m_Heads;
		qint32 m_SectorsPerTrack;
		qint32 m_Cylinders;
		qint32 m_LogicalSectorSize;
		qint32 m_PhysicalSectorSize;
		QString m_IconName;
		SmartStatus* m_SmartStatus;
};

#endif

