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

#include "core/copysourcefile.h"

#include <QFile>
#include <QFileInfo>

/** Constructs a CopySourceFile from the given @p filename.
	@param filename filename of the file to copy from
	@param sectorsize the sector size to assume for the file, usually the target Device's sector size
*/
CopySourceFile::CopySourceFile(const QString& filename, qint32 sectorsize) :
	CopySource(),
	m_File(filename),
	m_SectorSize(sectorsize)
{
}

/** Opens the file.
	@return true on success
*/
bool CopySourceFile::open()
{
	return file().open(QIODevice::ReadOnly);
}

/** Returns the length of the file in sectors.
	@return length of the file in sectors.
*/
qint64 CopySourceFile::length() const
{
	return QFileInfo(file()).size() / sectorSize();
}

/** Reads the given number of sectors from the file into the given buffer.
	@param buffer buffer to store the sectors read in
	@param readOffset offset where to begin reading
	@param numSectors number of sectors to read
	@return true on success
*/
bool CopySourceFile::readSectors(void* buffer, qint64 readOffset, qint64 numSectors)
{
	if (!file().seek(readOffset * sectorSize()))
		return false;

	return file().read(static_cast<char*>(buffer), numSectors * sectorSize()) == numSectors * sectorSize();
}
