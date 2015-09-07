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

#if !defined(COPYTARGET__H)

#define COPYTARGET__H

#include <qglobal.h>


/** Base class for something to copy to.

	Abstract base class for all copy targets. Used together with CopySource to
	implement moving, copying, restoring and backing up FileSystems.

	@see CopySource
	@author Volker Lanz <vl@fidra.de>
*/
class CopyTarget
{
	Q_DISABLE_COPY(CopyTarget)

	protected:
		CopyTarget() : m_SectorsWritten(0) {}
		virtual ~CopyTarget() {}

	public:
		virtual bool open() = 0;
		virtual qint32 sectorSize() const = 0;
		virtual bool writeSectors(void* buffer, qint64 writeOffset, qint64 numSectors) = 0;
		virtual qint64 firstSector() const = 0;
		virtual qint64 lastSector() const = 0;

		qint64 sectorsWritten() const { return m_SectorsWritten; }

	protected:
		void setSectorsWritten(qint64 s) { m_SectorsWritten = s; }
		
	private:
		qint64 m_SectorsWritten;
};

#endif
