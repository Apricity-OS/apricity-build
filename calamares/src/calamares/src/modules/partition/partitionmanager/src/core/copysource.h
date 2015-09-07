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

#if !defined(COPYSOURCE__H)

#define COPYSOURCE__H

#include <qglobal.h>

class CopyTarget;

/** Base class for something to copy from.

	Abstract base class for all copy sources. Used in combination with CopyTarget
	to implement moving, copying, backing up and restoring FileSystems.

	@see CopyTarget
	@author Volker Lanz <vl@fidra.de>
*/
class CopySource
{
	Q_DISABLE_COPY(CopySource)

	protected:
		CopySource() {}
		virtual ~CopySource() {}

	public:
		virtual bool open() = 0;
		virtual qint32 sectorSize() const = 0;
		virtual bool readSectors(void* buffer, qint64 readOffset, qint64 numSectors) = 0;
		virtual qint64 length() const = 0;
		virtual bool overlaps(const CopyTarget& target) const = 0;

		virtual qint64 firstSector() const = 0;
		virtual qint64 lastSector() const = 0;

	private:
};

#endif
