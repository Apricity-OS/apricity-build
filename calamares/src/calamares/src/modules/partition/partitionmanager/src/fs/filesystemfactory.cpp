/***************************************************************************
 *   Copyright (C) 2012 by Volker Lanz <vl@fidra.de>                       *
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

#include "fs/filesystemfactory.h"
#include "fs/filesystem.h"

#include "fs/btrfs.h"
#include "fs/exfat.h"
#include "fs/ext2.h"
#include "fs/ext3.h"
#include "fs/ext4.h"
#include "fs/extended.h"
#include "fs/fat16.h"
#include "fs/fat32.h"
#include "fs/hfs.h"
#include "fs/hfsplus.h"
#include "fs/hpfs.h"
#include "fs/jfs.h"
#include "fs/linuxswap.h"
#include "fs/luks.h"
#include "fs/lvm2_pv.h"
#include "fs/nilfs2.h"
#include "fs/ntfs.h"
#include "fs/ocfs2.h"
#include "fs/reiser4.h"
#include "fs/reiserfs.h"
#include "fs/ufs.h"
#include "fs/unformatted.h"
#include "fs/unknown.h"
#include "fs/xfs.h"
#include "fs/zfs.h"

#include "backend/corebackendmanager.h"
#include "backend/corebackend.h"

FileSystemFactory::FileSystems FileSystemFactory::m_FileSystems;

/** Initializes the instance. */
void FileSystemFactory::init()
{
	qDeleteAll(m_FileSystems);
	m_FileSystems.clear();

	m_FileSystems.insert(FileSystem::Btrfs, new FS::btrfs(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::Exfat, new FS::exfat(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::Ext2, new FS::ext2(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::Ext3, new FS::ext3(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::Ext4, new FS::ext4(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::Extended, new FS::extended(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::Fat16, new FS::fat16(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::Fat32, new FS::fat32(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::Hfs, new FS::hfs(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::HfsPlus, new FS::hfsplus(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::Hpfs, new FS::hpfs(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::Jfs, new FS::jfs(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::LinuxSwap, new FS::linuxswap(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::Luks, new FS::luks(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::Lvm2_PV, new FS::lvm2_pv(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::Nilfs2, new FS::nilfs2(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::Ntfs, new FS::ntfs(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::Ocfs2, new FS::ocfs2(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::ReiserFS, new FS::reiserfs(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::Reiser4, new FS::reiser4(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::Ufs, new FS::ufs(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::Unformatted, new FS::unformatted(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::Unknown, new FS::unknown(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::Xfs, new FS::xfs(-1, -1, -1, QString()));
	m_FileSystems.insert(FileSystem::Zfs, new FS::zfs(-1, -1, -1, QString()));

	FS::btrfs::init();
	FS::exfat::init();
	FS::ext2::init();
	FS::ext3::init();
	FS::ext4::init();
	FS::extended::init();
	FS::fat16::init();
	FS::fat32::init();
	FS::hfs::init();
	FS::hfsplus::init();
	FS::hpfs::init();
	FS::jfs::init();
	FS::linuxswap::init();
	FS::luks::init();
	FS::lvm2_pv::init();
	FS::nilfs2::init();
	FS::ntfs::init();
	FS::ocfs2::init();
	FS::reiserfs::init();
	FS::reiser4::init();
	FS::ufs::init();
	FS::unformatted::init();
	FS::unknown::init();
	FS::xfs::init();
	FS::zfs::init();

	CoreBackendManager::self()->backend()->initFSSupport();
}

/** Creates a new FileSystem
	@param t the FileSystem's type
	@param firstsector the FileSystem's first sector relative to the Device
	@param lastsector the FileSystem's last sector relative to the Device
	@param sectorsused the number of used sectors in the FileSystem
	@param label the FileSystem's label
	@return pointer to the newly created FileSystem object or NULL if FileSystem could not be created
*/
FileSystem* FileSystemFactory::create(FileSystem::Type t, qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label, const QString& uuid)
{
	FileSystem* fs = NULL;

	switch(t)
	{
		case FileSystem::Btrfs:        fs = new FS::btrfs(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::Exfat:        fs = new FS::exfat(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::Ext2:         fs = new FS::ext2(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::Ext3:         fs = new FS::ext3(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::Ext4:         fs = new FS::ext4(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::Extended:     fs = new FS::extended(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::Fat16:        fs = new FS::fat16(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::Fat32:        fs = new FS::fat32(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::Hfs:          fs = new FS::hfs(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::HfsPlus:      fs = new FS::hfsplus(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::Hpfs:         fs = new FS::hpfs(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::Jfs:          fs = new FS::jfs(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::LinuxSwap:    fs = new FS::linuxswap(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::Luks:         fs = new FS::luks(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::Lvm2_PV:      fs = new FS::lvm2_pv(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::Nilfs2:       fs = new FS::nilfs2(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::Ntfs:         fs = new FS::ntfs(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::Ocfs2:        fs = new FS::ocfs2(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::ReiserFS:     fs = new FS::reiserfs(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::Reiser4:      fs = new FS::reiser4(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::Ufs:          fs = new FS::ufs(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::Unformatted:  fs = new FS::unformatted(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::Unknown:      fs = new FS::unknown(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::Xfs:          fs = new FS::xfs(firstsector, lastsector, sectorsused, label); break;
		case FileSystem::Zfs:          fs = new FS::zfs(firstsector, lastsector, sectorsused, label); break;
		default:                       break;
	}

	if (fs != NULL)
		fs->setUUID(uuid);

	return fs;
}

/**
	@overload
*/
FileSystem* FileSystemFactory::create(const FileSystem& other)
{
	return create(other.type(), other.firstSector(), other.lastSector(), other.sectorsUsed(), other.label(), other.uuid());
}

/** @return the map of FileSystems */
const FileSystemFactory::FileSystems& FileSystemFactory::map()
{
	return m_FileSystems;
}

/** Clones a FileSystem from another one, but with a new type.
	@param newType the new FileSystem's type
	@param other the old FileSystem to clone
	@return pointer to the newly created FileSystem or NULL in case of errors
*/
FileSystem* FileSystemFactory::cloneWithNewType(FileSystem::Type newType, const FileSystem& other)
{
	return create(newType, other.firstSector(), other.lastSector(), other.sectorsUsed(), other.label());
}
