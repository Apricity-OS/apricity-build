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

#if !defined(COPYSOURCEDEVICE__H)

#define COPYSOURCEDEVICE__H

#include "core/copysource.h"

#include <qglobal.h>

class Device;
class CopyTarget;
class CoreBackendDevice;

/** A Device to copy from.

	Represents a Device to copy from. Used to copy a Partition to somewhere on the same or
	another Device or to backup its FileSystem to a file.
	@author Volker Lanz <vl@fidra.de>
 */
class CopySourceDevice : public CopySource
{
	Q_DISABLE_COPY(CopySourceDevice)

	public:
		CopySourceDevice(Device& d, qint64 firstsector, qint64 lastsector);
		~CopySourceDevice();

	public:
		virtual bool open();
		virtual qint32 sectorSize() const;
		virtual bool readSectors(void* buffer, qint64 readOffset, qint64 numSectors);
		virtual qint64 length() const;
		virtual bool overlaps(const CopyTarget& target) const;

		virtual qint64 firstSector() const { return m_FirstSector; } /**< @return first sector to copying */
		virtual qint64 lastSector() const { return m_LastSector; } /**< @return last sector to copy */

		Device& device() { return m_Device; } /**< @return Device to copy from */
		const Device& device() const { return m_Device; } /**< @return Device to copy from */

	protected:
		Device& m_Device;
		const qint64 m_FirstSector;
		const qint64 m_LastSector;
		CoreBackendDevice* m_BackendDevice
;
};

#endif
