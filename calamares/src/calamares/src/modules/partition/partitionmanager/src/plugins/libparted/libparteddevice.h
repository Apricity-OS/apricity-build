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

#if !defined(LIBPARTEDDEVICE__H)

#define LIBPARTEDDEVICE__H

#include "backend/corebackenddevice.h"

#include <qglobal.h>

#include <parted/parted.h>

class Partition;
class PartitionTable;
class Report;
class CoreBackendPartitionTable;

class LibPartedDevice : public CoreBackendDevice
{
	Q_DISABLE_COPY(LibPartedDevice);

	public:
		LibPartedDevice(const QString& device_node);
		~LibPartedDevice();

	public:
		virtual bool open();
		virtual bool openExclusive();
		virtual bool close();

		virtual CoreBackendPartitionTable* openPartitionTable();

		virtual bool createPartitionTable(Report& report, const PartitionTable& ptable);

		virtual bool readSectors(void* buffer, qint64 offset, qint64 numSectors);
		virtual bool writeSectors(void* buffer, qint64 offset, qint64 numSectors);

	protected:
		PedDevice* pedDevice() { return m_PedDevice; }

	private:
		PedDevice* m_PedDevice;
};

#endif
