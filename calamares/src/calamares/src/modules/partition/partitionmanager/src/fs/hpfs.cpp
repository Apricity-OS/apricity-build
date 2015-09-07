/***************************************************************************
 *   Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                       *
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

#include "fs/hpfs.h"

#include "util/capacity.h"

#include <QString>

namespace FS
{
	FileSystem::CommandSupportType hpfs::m_GetUsed = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType hpfs::m_GetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType hpfs::m_Create = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType hpfs::m_Grow = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType hpfs::m_Shrink = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType hpfs::m_Move = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType hpfs::m_Check = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType hpfs::m_Copy = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType hpfs::m_Backup = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType hpfs::m_SetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType hpfs::m_UpdateUUID = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType hpfs::m_GetUUID = FileSystem::cmdSupportNone;

	hpfs::hpfs(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Hpfs)
	{
	}

	void hpfs::init()
	{
	}

	qint64 hpfs::maxCapacity() const
	{
		 return 2 * Capacity::unitFactor(Capacity::Byte, Capacity::TiB);
	}
}
