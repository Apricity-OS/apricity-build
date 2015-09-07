/***************************************************************************
 *   Copyright (C) 2008,2009,2010,2011,2012 by Volker Lanz <vl@fidra.de>   *
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

/** @file
*/

#include "plugins/libparted/libpartedbackend.h"
#include "plugins/libparted/libparteddevice.h"

#include "core/device.h"
#include "core/partition.h"
#include "core/partitiontable.h"
#include "core/partitionalignment.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"

#include "fs/fat16.h"
#include "fs/hfs.h"
#include "fs/hfsplus.h"
#include "fs/luks.h"

#include "util/globallog.h"
#include "util/helpers.h"

#include <QString>
#include <QStringList>
#include <QDebug>

#include <KLocalizedString>
#ifdef CALAMARES
// Those files are local copies to avoid the dependency on KIO
#include <kio/kmountpoint.h>
#include <kio/kdiskfreespaceinfo.h>
#else
#include <KIOCore/KMountPoint>
#include <KIOCore/KDiskFreeSpaceInfo>
#endif

#ifndef CALAMARES
#include <KPluginFactory>
#include <KAboutData>
#endif

#include <parted/parted.h>
#include <unistd.h>
#include <blkid/blkid.h>

#ifndef CALAMARES
K_PLUGIN_FACTORY(LibPartedBackendFactory, registerPlugin<LibPartedBackend>(); )

static KAboutData createPluginAboutData()
{
	KAboutData about(
		QStringLiteral("pmlibpartedbackendplugin"),
		i18nc("@title", "LibParted Backend Plugin"),
		QStringLiteral("%1, libparted version: %2").arg(QString::fromLatin1(VERSION)).arg(QString::fromLatin1(ped_get_version())),
		i18n("KDE Partition Manager backend for libparted."),
		KAboutLicense::GPL,
		i18n("Copyright 2008,2009,2010 Volker Lanz"));

	about.addAuthor(i18nc("@info:credit", "Volker Lanz"), i18nc("@info:credit", "Former maintainer"));
	about.setHomepage(QStringLiteral("http://www.partitionmanager.org"));

	return about;
}
#endif

static struct
{
	PedPartitionFlag pedFlag;
	PartitionTable::Flag flag;
} flagmap[] =
{
	{ PED_PARTITION_BOOT, PartitionTable::FlagBoot },
	{ PED_PARTITION_ROOT, PartitionTable::FlagRoot },
	{ PED_PARTITION_SWAP, PartitionTable::FlagSwap },
	{ PED_PARTITION_HIDDEN, PartitionTable::FlagHidden },
	{ PED_PARTITION_RAID, PartitionTable::FlagRaid },
	{ PED_PARTITION_LVM, PartitionTable::FlagLvm },
	{ PED_PARTITION_LBA, PartitionTable::FlagLba },
	{ PED_PARTITION_HPSERVICE, PartitionTable::FlagHpService },
	{ PED_PARTITION_PALO, PartitionTable::FlagPalo },
	{ PED_PARTITION_PREP, PartitionTable::FlagPrep },
	{ PED_PARTITION_MSFT_RESERVED, PartitionTable::FlagMsftReserved }
};

static QString s_lastPartedExceptionMessage;

/** Callback to handle exceptions from libparted
	@param e the libparted exception to handle
*/
static PedExceptionOption pedExceptionHandler(PedException* e)
{
	Log(Log::error) << i18nc("@info/plain", "LibParted Exception: %1", QString::fromLocal8Bit(e->message));
	s_lastPartedExceptionMessage = QString::fromLocal8Bit(e->message);
	return PED_EXCEPTION_UNHANDLED;
}

// --------------------------------------------------------------------------

// The following structs and the typedef come from libparted's internal gpt sources.
// It's very unfortunate there is no public API to get at the first and last usable
// sector for GPT a partition table, so this is the only (libparted) way to get that
// information (another way would be to read the GPT header and parse the
// information ourselves; if the libparted devs begin changing these internal
// structs for each point release and break our code, we'll have to do just that).

typedef struct {
        uint32_t time_low;
        uint16_t time_mid;
        uint16_t time_hi_and_version;
        uint8_t  clock_seq_hi_and_reserved;
        uint8_t  clock_seq_low;
        uint8_t  node[6];
} /* __attribute__ ((packed)) */ efi_guid_t;


struct __attribute__ ((packed)) _GPTDiskData {
    PedGeometry data_area;
    int     entry_count;
    efi_guid_t  uuid;
};

typedef struct _GPTDiskData GPTDiskData;

// --------------------------------------------------------------------------

/** Get the first sector a Partition may cover on a given Device
    @param d the Device in question
    @return the first sector usable by a Partition
*/
static quint64 firstUsableSector(const Device& d)
{
	PedDevice* pedDevice = ped_device_get(d.deviceNode().toLatin1().constData());
	PedDisk* pedDisk = pedDevice ? ped_disk_new(pedDevice) : NULL;

	quint64 rval = pedDisk->dev->bios_geom.sectors;

	if (pedDisk && strcmp(pedDisk->type->name, "gpt") == 0)
	{
		GPTDiskData* gpt_disk_data = reinterpret_cast<GPTDiskData*>(pedDisk->disk_specific);
		PedGeometry* geom = reinterpret_cast<PedGeometry*>(&gpt_disk_data->data_area);

		if (geom)
			rval = geom->start;
		else
			rval += 32;
	}

	return rval;
}

/** Get the last sector a Partition may cover on a given Device
    @param d the Device in question
    @return the last sector usable by a Partition
*/
static quint64 lastUsableSector(const Device& d)
{
	PedDevice* pedDevice = ped_device_get(d.deviceNode().toLatin1().constData());
	PedDisk* pedDisk = pedDevice ? ped_disk_new(pedDevice) : NULL;

	quint64 rval = pedDisk->dev->bios_geom.sectors * pedDisk->dev->bios_geom.heads * pedDisk->dev->bios_geom.cylinders - 1;

	if (pedDisk && strcmp(pedDisk->type->name, "gpt") == 0)
	{
		GPTDiskData* gpt_disk_data = reinterpret_cast<GPTDiskData*>(pedDisk->disk_specific);
		PedGeometry* geom = reinterpret_cast<PedGeometry*>(&gpt_disk_data->data_area);

		if (geom)
			rval = geom->end;
		else
			rval -= 32;
	}

	return rval;
}

/** Reads sectors used on a FileSystem using libparted functions.
	@param pedDisk pointer to pedDisk  where the Partition and its FileSystem are
	@param p the Partition the FileSystem is on
	@return the number of sectors used
*/
#if defined LIBPARTED_FS_RESIZE_LIBRARY_SUPPORT
static qint64 readSectorsUsedLibParted(PedDisk* pedDisk, const Partition& p)
{
	Q_ASSERT(pedDisk);

	qint64 rval = -1;

	PedPartition* pedPartition = ped_disk_get_partition_by_sector(pedDisk, p.firstSector());

	if (pedPartition)
	{
		PedFileSystem* pedFileSystem = ped_file_system_open(&pedPartition->geom);

		if (pedFileSystem)
		{
			if (PedConstraint* pedConstraint = ped_file_system_get_resize_constraint(pedFileSystem))
			{
				rval = pedConstraint->min_size;
				ped_constraint_destroy(pedConstraint);
			}

			ped_file_system_close(pedFileSystem);
		}
	}

	return rval;
}
#endif

/** Reads the sectors used in a FileSystem and stores the result in the Partition's FileSystem object.
	@param pedDisk pointer to pedDisk  where the Partition and its FileSystem are
	@param p the Partition the FileSystem is on
	@param mountPoint mount point of the partition in question
*/
static void readSectorsUsed(PedDisk* pedDisk, const Device& d, Partition& p, const QString& mountPoint)
{
	Q_ASSERT(pedDisk);
    qDebug() << "readSectorsUsed for partition" << p.partitionPath();

	const KDiskFreeSpaceInfo freeSpaceInfo = KDiskFreeSpaceInfo::freeSpaceInfo(mountPoint);

	if (p.isMounted() && freeSpaceInfo.isValid())
		p.fileSystem().setSectorsUsed(freeSpaceInfo.used() / d.logicalSectorSize());
	else if (p.fileSystem().supportGetUsed() == FileSystem::cmdSupportFileSystem)
		p.fileSystem().setSectorsUsed(p.fileSystem().readUsedCapacity(p.deviceNode()) / d.logicalSectorSize());
#if defined LIBPARTED_FS_RESIZE_LIBRARY_SUPPORT
	else if (p.fileSystem().supportGetUsed() == FileSystem::cmdSupportCore)
		p.fileSystem().setSectorsUsed(readSectorsUsedLibParted(pedDisk, p));
#else
	Q_UNUSED(pedDisk);
#endif
}

static PartitionTable::Flags activeFlags(PedPartition* p)
{
	PartitionTable::Flags flags = PartitionTable::FlagNone;

	// We might get here with a pedPartition just picked up from libparted that is
	// unallocated. Libparted doesn't like it if we ask for flags for unallocated
	// space.
	if (p->num <= 0)
		return flags;

	for (quint32 i = 0; i < sizeof(flagmap) / sizeof(flagmap[0]); i++)
		if (ped_partition_is_flag_available(p, flagmap[i].pedFlag) && ped_partition_get_flag(p, flagmap[i].pedFlag))
			flags |= flagmap[i].flag;

	return flags;
}

static PartitionTable::Flags availableFlags(PedPartition* p)
{
	PartitionTable::Flags flags;

	// see above.
	if (p->num <= 0)
		return flags;

	for (quint32 i = 0; i < sizeof(flagmap) / sizeof(flagmap[0]); i++)
		if (ped_partition_is_flag_available(p, flagmap[i].pedFlag))
		{
			// Workaround: libparted claims the hidden flag is available for extended partitions, but
			// throws an error when we try to set or clear it. So skip this combination. Also see setFlag.
			if (p->type != PED_PARTITION_EXTENDED || flagmap[i].flag != PartitionTable::FlagHidden)
				flags |= flagmap[i].flag;
		}

	return flags;
}

/** Constructs a LibParted object. */
LibPartedBackend::LibPartedBackend(QObject*, const QList<QVariant>&) :
	CoreBackend()
{
	ped_exception_set_handler(pedExceptionHandler);
}

void LibPartedBackend::initFSSupport()
{
#if defined LIBPARTED_FS_RESIZE_LIBRARY_SUPPORT
	if (FS::fat16::m_Shrink == FileSystem::cmdSupportNone)
		FS::fat16::m_Shrink = FileSystem::cmdSupportBackend;

	if (FS::fat16::m_Grow == FileSystem::cmdSupportNone)
		FS::fat16::m_Grow = FileSystem::cmdSupportBackend;

	if (FS::hfs::m_Shrink == FileSystem::cmdSupportNone)
		FS::hfs::m_Shrink = FileSystem::cmdSupportBackend;

	if (FS::hfsplus::m_Shrink == FileSystem::cmdSupportNone)
		FS::hfsplus::m_Shrink = FileSystem::cmdSupportBackend;

	if (FS::hfs::m_GetUsed == FileSystem::cmdSupportNone)
		FS::hfs::m_GetUsed = FileSystem::cmdSupportBackend;

	if (FS::hfsplus::m_GetUsed == FileSystem::cmdSupportNone)
		FS::hfsplus::m_GetUsed = FileSystem::cmdSupportBackend;
#endif
}

/** Scans a Device for Partitions.

	This method  will scan a Device for all Partitions on it, detect the FileSystem for each Partition,
	try to determine the FileSystem usage, read the FileSystem label and store it all in newly created
	objects that are in the end added to the Device's PartitionTable.

	@param pedDevice libparted pointer to the device
	@param d Device
	@param pedDisk libparted pointer to the partition table
*/
void LibPartedBackend::scanDevicePartitions(PedDevice*, Device& d, PedDisk* pedDisk)
{
	Q_ASSERT(pedDisk);
	Q_ASSERT(d.partitionTable());

    qDebug() << "LibPartedBackend::scanDevicePartitions for" << d.deviceNode();

	PedPartition* pedPartition = NULL;

	KMountPoint::List mountPoints = KMountPoint::currentMountPoints(KMountPoint::NeedRealDeviceName);
	mountPoints.append(KMountPoint::possibleMountPoints(KMountPoint::NeedRealDeviceName));

	QList<Partition*> partitions;

    qDebug() << "LibParted iterating over pedPartitions on the pedDisk";
	while ((pedPartition = ped_disk_next_partition(pedDisk, pedPartition)))
	{
		if (pedPartition->num < 1)
			continue;

		PartitionRole::Roles r = PartitionRole::None;
		FileSystem::Type type = detectFileSystem(pedPartition);

        qDebug() << "pedPartition number:" << pedPartition->num;
        qDebug() << "pedPartition type:" << pedPartition->type;
        qDebug() << "Detected file system type" << type << ":" << FileSystem::nameForType(type);
		switch(pedPartition->type)
		{
			case PED_PARTITION_NORMAL:
				r = PartitionRole::Primary;
				break;

			case PED_PARTITION_EXTENDED:
				r = PartitionRole::Extended;
				type = FileSystem::Extended;
				break;

			case PED_PARTITION_LOGICAL:
				r = PartitionRole::Logical;
				break;

			default:
				continue;
		}

		// Find an extended partition this partition is in.
		PartitionNode* parent = d.partitionTable()->findPartitionBySector(pedPartition->geom.start, PartitionRole(PartitionRole::Extended));

		// None found, so it's a primary in the device's partition table.
		if (parent == NULL)
			parent = d.partitionTable();

		const QString node = QString::fromUtf8(ped_partition_get_path(pedPartition));
		FileSystem* fs = FileSystemFactory::create(type, pedPartition->geom.start, pedPartition->geom.end);

        qDebug() << "Partition path:" << node;
        qDebug() << "FileSystemFactory::create returned"
                 << (fs ? "a valid FileSystem." : "nullptr!");

		// libparted does not handle LUKS partitions
		QString mountPoint;
		bool mounted;
		if (fs->type() == FileSystem::Luks)
		{
			mountPoint = FS::luks::mapperName(node);
			mounted = (mountPoint != QString()) ? true : false;
		}
		else
		{
			mountPoint = mountPoints.findByDevice(node) ? mountPoints.findByDevice(node)->mountPoint() : QString();
			mounted = ped_partition_is_busy(pedPartition);
		}

        qDebug() << "mountPoint:" << mountPoint;
        qDebug() << "mounted:" << mounted;

		Partition* part = new Partition(parent, d, PartitionRole(r), fs, pedPartition->geom.start, pedPartition->geom.end, node, availableFlags(pedPartition), mountPoint, mounted, activeFlags(pedPartition));

        qDebug() << "Partition object is" << (part ? "not null." : "null.");

		readSectorsUsed(pedDisk, d, *part, mountPoint);

		if (fs->supportGetLabel() != FileSystem::cmdSupportNone)
			fs->setLabel(fs->readLabel(part->deviceNode()));

		if (fs->supportGetUUID() != FileSystem::cmdSupportNone)
			fs->setUUID(fs->readUUID(part->deviceNode()));

		parent->append(part);
		partitions.append(part);
	}

	d.partitionTable()->updateUnallocated(d);

	if (d.partitionTable()->isSectorBased(d))
		d.partitionTable()->setType(d, PartitionTable::msdos_sectorbased);

	foreach(const Partition* part, partitions)
		PartitionAlignment::isAligned(d, *part);

	ped_disk_destroy(pedDisk);
}

/** Create a Device for the given device_node and scan it for partitions.
	@param device_node the device node (e.g. "/dev/sda")
	@return the created Device object. callers need to free this.
*/
Device* LibPartedBackend::scanDevice(const QString& device_node)
{
    qDebug() << "LibPartedBackend::scanDevice for" << device_node;
	PedDevice* pedDevice = ped_device_get(device_node.toLocal8Bit().constData());

	if (pedDevice == NULL)
	{
		Log(Log::warning) << xi18nc("@info/plain", "Could not access device <filename>%1</filename>", device_node);
		return NULL;
	}

	Log(Log::information) << i18nc("@info/plain", "Device found: %1", QString::fromUtf8(pedDevice->model));

	Device* d = new Device(QString::fromUtf8(pedDevice->model), QString::fromUtf8(pedDevice->path), pedDevice->bios_geom.heads, pedDevice->bios_geom.sectors, pedDevice->bios_geom.cylinders, pedDevice->sector_size);

	PedDisk* pedDisk = ped_disk_new(pedDevice);

	if (pedDisk)
	{
		const PartitionTable::TableType type = PartitionTable::nameToTableType(QString::fromUtf8(pedDisk->type->name));
		CoreBackend::setPartitionTableForDevice(*d, new PartitionTable(type, firstUsableSector(*d), lastUsableSector(*d)));
		CoreBackend::setPartitionTableMaxPrimaries(*d->partitionTable(), ped_disk_get_max_primary_partition_count(pedDisk));

		scanDevicePartitions(pedDevice, *d, pedDisk);
	}

	return d;
}

QList<Device*> LibPartedBackend::scanDevices()
{
	QList<Device*> result;

	ped_device_probe_all();
	PedDevice* pedDevice = NULL;
	while (true)
	{
		pedDevice = ped_device_get_next(pedDevice);
		if (!pedDevice)
			break;
        if (pedDevice->type == PED_DEVICE_DM ||
            pedDevice->type == PED_DEVICE_LOOP ||
            pedDevice->type == PED_DEVICE_UNKNOWN )
			continue;
        if (pedDevice->read_only)
            continue;
		QString path = QString::fromUtf8(pedDevice->path);
		Device* d = scanDevice(path);
		if (d)
			result.append(d);
	}

	return result;
}

CoreBackendDevice* LibPartedBackend::openDevice(const QString& device_node)
{
	LibPartedDevice* device = new LibPartedDevice(device_node);

	if (device == NULL || !device->open())
	{
		delete device;
		device = NULL;
	}

	return device;
}

CoreBackendDevice* LibPartedBackend::openDeviceExclusive(const QString& device_node)
{
	LibPartedDevice* device = new LibPartedDevice(device_node);

	if (device == NULL || !device->openExclusive())
	{
		delete device;
		device = NULL;
	}

	return device;
}

bool LibPartedBackend::closeDevice(CoreBackendDevice* core_device)
{
	return core_device->close();
}

/** Detects the type of a FileSystem given a PedDevice and a PedPartition
	@param pedDevice pointer to the pedDevice. Must not be NULL.
	@param pedPartition pointer to the pedPartition. Must not be NULL
	@return the detected FileSystem type (FileSystem::Unknown if not detected)
*/
FileSystem::Type LibPartedBackend::detectFileSystem(PedPartition* pedPartition)
{
	FileSystem::Type rval = FileSystem::Unknown;

	blkid_cache cache;
	char* pedPath = NULL;

	if (blkid_get_cache(&cache, NULL) == 0 && (pedPath = ped_partition_get_path(pedPartition)))
	{
		blkid_dev dev;

		if ((dev = blkid_get_dev(cache, pedPath, BLKID_DEV_NORMAL)) != NULL)
		{
			QString s = QString::fromUtf8(blkid_get_tag_value(cache, "TYPE", pedPath));

			if (s == QStringLiteral("ext2")) rval = FileSystem::Ext2;
			else if (s == QStringLiteral("ext3")) rval = FileSystem::Ext3;
			else if (s.startsWith(QStringLiteral("ext4"))) rval = FileSystem::Ext4;
			else if (s == QStringLiteral("swap")) rval = FileSystem::LinuxSwap;
			else if (s == QStringLiteral("ntfs")) rval = FileSystem::Ntfs;
			else if (s == QStringLiteral("reiserfs")) rval = FileSystem::ReiserFS;
			else if (s == QStringLiteral("reiser4")) rval = FileSystem::Reiser4;
			else if (s == QStringLiteral("xfs")) rval = FileSystem::Xfs;
			else if (s == QStringLiteral("jfs")) rval = FileSystem::Jfs;
			else if (s == QStringLiteral("hfs")) rval = FileSystem::Hfs;
			else if (s == QStringLiteral("hfsplus")) rval = FileSystem::HfsPlus;
			else if (s == QStringLiteral("ufs")) rval = FileSystem::Ufs;
			else if (s == QStringLiteral("vfat") && pedPartition->fs_type != NULL)
			{
				// libblkid does not distinguish between fat16 and fat32, so we're still using libparted
				// for those
				if (strcmp(pedPartition->fs_type->name, "fat16") == 0)
					rval = FileSystem::Fat16;
				else if (strcmp(pedPartition->fs_type->name, "fat32") == 0)
					rval = FileSystem::Fat32;
			}
			else if (s == QStringLiteral("btrfs")) rval = FileSystem::Btrfs;
			else if (s == QStringLiteral("ocfs2")) rval = FileSystem::Ocfs2;
			else if (s == QStringLiteral("zfs_member")) rval = FileSystem::Zfs;
			else if (s == QStringLiteral("hpfs")) rval = FileSystem::Hpfs;
			else if (s == QStringLiteral("crypto_LUKS")) rval = FileSystem::Luks;
			else if (s == QStringLiteral("exfat")) rval = FileSystem::Exfat;
			else if (s == QStringLiteral("nilfs2")) rval = FileSystem::Nilfs2;
			else if (s == QStringLiteral("LVM2_member")) rval = FileSystem::Lvm2_PV;
			else
				qWarning() << "blkid: unknown file system type " << s << " on " << pedPath;
		}

		blkid_put_cache(cache);

		free(pedPath);
	}

	return rval;
}

PedPartitionFlag LibPartedBackend::getPedFlag(PartitionTable::Flag flag)
{
	for (quint32 i = 0; i < sizeof(flagmap) / sizeof(flagmap[0]); i++)
		if (flagmap[i].flag == flag)
			return flagmap[i].pedFlag;

	return static_cast<PedPartitionFlag>(-1);
}

QString LibPartedBackend::lastPartedExceptionMessage()
{
	return s_lastPartedExceptionMessage;
}

#include "libpartedbackend.moc"
