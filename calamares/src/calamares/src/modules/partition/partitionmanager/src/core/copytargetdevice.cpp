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

#include "core/copytargetdevice.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"
#include "backend/corebackenddevice.h"

#include "core/device.h"


/** Constructs a device to copy to.
	@param d the Device to copy to
	@param firstsector the first sector on the Device to write to
	@param lastsector the last sector on the Device to write to
*/
CopyTargetDevice::CopyTargetDevice(Device& d, qint64 firstsector, qint64 lastsector) :
	CopyTarget(),
	m_Device(d),
	m_BackendDevice(NULL),
	m_FirstSector(firstsector),
	m_LastSector(lastsector)
{
}

/** Destructs a CopyTargetDevice */
CopyTargetDevice::~CopyTargetDevice()
{
	delete m_BackendDevice;
}

/** Opens a CopyTargetDevice for writing to.
	@return true on success
*/
bool CopyTargetDevice::open()
{
	m_BackendDevice = CoreBackendManager::self()->backend()->openDeviceExclusive(device().deviceNode());
	return m_BackendDevice != NULL;
}

/** @return the Device's sector size */
qint32 CopyTargetDevice::sectorSize() const
{
	return device().logicalSectorSize();
}

/** Writes the given number of sectors to the Device.

	Note that @p writeOffset must be greater or equal than zero.

	@param buffer the data to write
	@param writeOffset where to start writing on the Device
	@param numSectors the number of sectors in @p buffer
	@return true on success
*/
bool CopyTargetDevice::writeSectors(void* buffer, qint64 writeOffset, qint64 numSectors)
{
	Q_ASSERT(writeOffset >= 0);
	bool rval = m_BackendDevice->writeSectors(buffer, writeOffset, numSectors);

	if (rval)
		setSectorsWritten(sectorsWritten() + numSectors);

	return rval;
}
