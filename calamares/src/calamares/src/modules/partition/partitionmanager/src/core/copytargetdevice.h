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

#if !defined(COPYTARGETDEVICE__H)

#define COPYTARGETDEVICE__H

#include "core/copytarget.h"

#include <qglobal.h>

class Device;
class CoreBackendDevice;

/** A Device to copy to.

	Represents a target Device to copy to. Used to copy a Partition to somewhere on the same
	or another Device or to restore a FileSystem from a file to a Partition.

	@see CopyTargetFile, CopySourceDevice

	@author Volker Lanz <vl@fidra.de>
*/
class CopyTargetDevice : public CopyTarget
{
	Q_DISABLE_COPY(CopyTargetDevice)

	public:
		CopyTargetDevice(Device& d, qint64 firstsector, qint64 lastsector);
		~CopyTargetDevice();

	public:
		virtual bool open();
		virtual qint32 sectorSize() const;
		virtual bool writeSectors(void* buffer, qint64 writeOffset, qint64 numSectors);
		virtual qint64 firstSector() const { return m_FirstSector; } /**< @return the first sector to write to */
		virtual qint64 lastSector() const { return m_LastSector; } /**< @return the last sector to write to */

		Device& device() { return m_Device; } /**< @return the Device to write to */
		const Device& device() const { return m_Device; } /**< @return the Device to write to */

	protected:
		Device& m_Device;
		CoreBackendDevice* m_BackendDevice;
		const qint64 m_FirstSector;
		const qint64 m_LastSector;
};

#endif
