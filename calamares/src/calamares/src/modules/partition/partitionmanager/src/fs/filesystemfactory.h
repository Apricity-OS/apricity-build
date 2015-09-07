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

#if !defined(FILESYSTEMFACTORY__H)

#define FILESYSTEMFACTORY__H

#include "fs/filesystem.h"

#include "util/libpartitionmanagerexport.h"

#include <QMap>
#include <qglobal.h>

class QString;

/** Factory to create instances of FileSystem.
	@author Volker Lanz <vl@fidra.de>
 */
class LIBPARTITIONMANAGERPRIVATE_EXPORT FileSystemFactory
{
	public:
		/** map of FileSystem::Types to pointers of FileSystem */
		typedef QMap<FileSystem::Type, FileSystem*> FileSystems;

	private:
		FileSystemFactory();

	public:
		static void init();
		static FileSystem* create(FileSystem::Type t, qint64 firstsector, qint64 lastsector, qint64 sectorsused = -1, const QString& label = QString(), const QString& uuid = QString());
		static FileSystem* create(const FileSystem& other);
		static FileSystem* cloneWithNewType(FileSystem::Type newType, const FileSystem& other);
		static const FileSystems& map();

	private:
		static FileSystems m_FileSystems;
};

#endif
