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

#if !defined(COPYSOURCEFILE__H)

#define COPYSOURCEFILE__H

#include "core/copysource.h"

#include <qglobal.h>
#include <QFile>

class QString;
class CopyTarget;

/** A file to copy from.

	Represents a file to copy from. Used to restore a FileSystem from a backup file.

	@author Volker Lanz <vl@fidra.de>
*/
class CopySourceFile : public CopySource
{
	public:
		CopySourceFile(const QString& filename, qint32 sectorsize);

	public:
		virtual bool open();
		virtual bool readSectors(void* buffer, qint64 readOffset, qint64 numSectors);
		virtual qint64 length() const;
		
		virtual qint32 sectorSize() const { return m_SectorSize; } /**< @return the file's sector size */
		virtual bool overlaps(const CopyTarget&) const { return false; } /**< @return false for file */
		virtual qint64 firstSector() const { return 0; } /**< @return 0 for file */
		virtual qint64 lastSector() const { return length(); } /**< @return equal to length for file. @see length() */
		
	protected:
		QFile& file() { return m_File; }
		const QFile& file() const { return m_File; }

	protected:
		QFile m_File;
		qint32 m_SectorSize;
};

#endif
