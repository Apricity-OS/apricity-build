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

#include "core/copytargetfile.h"

/** Constructs a file to write to.
	@param filename name of the file to write to
	@param sectorsize the "sector size" of the file to write to, usually the sector size of the CopySourceDevice
*/
CopyTargetFile::CopyTargetFile(const QString& filename, qint32 sectorsize) :
	CopyTarget(),
	m_File(filename),
	m_SectorSize(sectorsize)
{
}

/** Opens the file for writing.
	@return true on success
*/
bool CopyTargetFile::open()
{
	return file().open(QIODevice::WriteOnly | QIODevice::Truncate);
}

/** Writes the given number of sectors from the given buffer to the file.
	@param buffer the data to write
	@param writeOffset where in the file to start writing
	@param numSectors the number of sectors to write
	@return true on success
*/
bool CopyTargetFile::writeSectors(void* buffer, qint64 writeOffset, qint64 numSectors)
{
	if (!file().seek(writeOffset * sectorSize()))
		return false;

	bool rval = file().write(static_cast<char*>(buffer), numSectors * sectorSize()) == numSectors * sectorSize();

	if (rval)
		setSectorsWritten(sectorsWritten() + numSectors);

	return rval;
}
