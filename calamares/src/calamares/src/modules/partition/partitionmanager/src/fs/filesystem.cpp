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

#include "fs/filesystem.h"

#include "util/externalcommand.h"
#include "util/capacity.h"

#include <blkid/blkid.h>
#include <KLocalizedString>

#include <config.h>

/** Creates a new FileSystem object
	@param firstsector the first sector used by this FileSystem on the Device
	@param lastsector the last sector used by this FileSystem on the Device
	@param sectorsused the number of sectors in use on the FileSystem
	@param l the FileSystem label
	@param t the FileSystem type
*/
FileSystem::FileSystem(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& l, FileSystem::Type t) :
	m_Type(t),
	m_FirstSector(firstsector),
	m_LastSector(lastsector),
	m_SectorsUsed(sectorsused),
	m_Label(l),
	m_UUID()
{
}

/** Reads the capacity in use on this FileSystem
	@param deviceNode the device node for the Partition the FileSystem is on
	@return the used capacity in bytes or -1 in case of an error
*/
qint64 FileSystem::readUsedCapacity(const QString& deviceNode) const
{
	Q_UNUSED(deviceNode);

	return -1;
}

static QString readBlkIdValue(const QString& deviceNode, const QString& tag)
{
	blkid_cache cache;
	QString rval;

	if (blkid_get_cache(&cache, NULL) == 0)
	{
		blkid_dev dev;

		char* label = NULL;
		if ((dev = blkid_get_dev(cache, deviceNode.toLocal8Bit().constData(), BLKID_DEV_NORMAL)) != NULL &&
				(label = blkid_get_tag_value(cache, tag.toLocal8Bit().constData(), deviceNode.toLocal8Bit().constData())))
		{
			rval = QString::fromUtf8(label);
			free(label);
		}

		blkid_put_cache(cache);
	}

	return rval;
}

/** Reads the label for this FileSystem
	@param deviceNode the device node for the Partition the FileSystem is on
	@return the FileSystem label or an empty string in case of error
*/
QString FileSystem::readLabel(const QString& deviceNode) const
{
	return readBlkIdValue(deviceNode, QStringLiteral("LABEL"));
}

/** Creates a new FileSystem
	@param report Report to write status information to
	@param deviceNode the device node for the Partition to create the FileSystem on
	@return true if successful
*/
bool FileSystem::create(Report& report, const QString& deviceNode) const
{
	Q_UNUSED(report);
	Q_UNUSED(deviceNode);

	return true;
}

/** Resized a FileSystem to a given new length
	@param report Report to write status information to
	@param deviceNode the device node for the Partition the FileSystem is on
	@param newLength the new length for the FileSystem in bytes
	@return true on success
*/
bool FileSystem::resize(Report& report, const QString& deviceNode, qint64 newLength) const
{
	Q_UNUSED(report);
	Q_UNUSED(deviceNode);
	Q_UNUSED(newLength);

	return true;
}

/** Move a FileSystem to a new start sector
	@param report Report to write status information to
	@param deviceNode the device node for the Partition the FileSystem is on
	@param newLength the new start sector for the FileSystem
	@return true on success
*/
bool FileSystem::move(Report& report, const QString& deviceNode, qint64 newStartSector) const
{
	Q_UNUSED(report);
	Q_UNUSED(deviceNode);
	Q_UNUSED(newStartSector);

	return true;
}

/** Writes a label for the FileSystem to disk
	@param report Report to write status information to
	@param deviceNode the device node for the Partition the FileSystem is on
	@param newLabel the new label for the FileSystem
	@return true on success
*/
bool FileSystem::writeLabel(Report& report, const QString& deviceNode, const QString& newLabel)
{
	Q_UNUSED(report);
	Q_UNUSED(deviceNode);
	Q_UNUSED(newLabel);

	return true;
}

/** Copies a FileSystem from one Partition to another
	@param report Report to write status information to
	@param targetDeviceNode device node of the target Partition
	@param sourceDeviceNode device node of the source Partition
	@return true on success
*/
bool FileSystem::copy(Report& report, const QString& targetDeviceNode, const QString& sourceDeviceNode) const
{
	Q_UNUSED(report);
	Q_UNUSED(targetDeviceNode);
	Q_UNUSED(sourceDeviceNode);

	return true;
}

/** Backs up a FileSystem to a file
	@param report Report to write status information to
	@param sourceDevice Device the source FileSystem is on
	@param deviceNode device node of the source Partition
	@param filename name of the file to backup to
	@return true on success
*/
bool FileSystem::backup(Report& report, const Device& sourceDevice, const QString& deviceNode, const QString& filename) const
{
	Q_UNUSED(report);
	Q_UNUSED(sourceDevice);
	Q_UNUSED(deviceNode);
	Q_UNUSED(filename);

	return false;
}

/** Removes a FileSystem
	@param report Report to write status information to
	@param deviceNode the device node for the Partition the FileSystem is on
	@return true if FileSystem is removed
*/
bool FileSystem::remove(Report& report, const QString& deviceNode) const
{
	Q_UNUSED(report);
	Q_UNUSED(deviceNode);

	return true;
}

/** Checks a FileSystem for errors
	@param report Report to write status information to
	@param deviceNode the device node for the Partition the FileSystem is on
	@return true if FileSystem is error-free
*/
bool FileSystem::check(Report& report, const QString& deviceNode) const
{
	Q_UNUSED(report);
	Q_UNUSED(deviceNode);

	return true;
}

/** Updates a FileSystem UUID on disk
	@param report Report to write status information to
	@param deviceNode the device node for the Partition the FileSystem is on
	@return true on success
*/
bool FileSystem::updateUUID(Report& report, const QString& deviceNode) const
{
	Q_UNUSED(report);
	Q_UNUSED(deviceNode);

	return true;
}

/** Returns the FileSystem UUID by calling a FileSystem-specific helper program
	@param deviceNode the device node for the Partition the FileSystem is on
	@return the UUID or an empty string if the FileSystem does not support UUIDs
 */
QString FileSystem::readUUID(const QString& deviceNode) const
{
	return readBlkIdValue(deviceNode, QStringLiteral("UUID"));
}

/** Give implementations of FileSystem a chance to update the boot sector after the
	file system has been moved or copied.
	@param report Report to write status information to
	@param deviceNode the device node for the Partition the FileSystem is on
	@return true on success
*/
bool FileSystem::updateBootSector(Report& report, const QString& deviceNode) const
{
	Q_UNUSED(report);
	Q_UNUSED(deviceNode);

	return true;
}

/** @return the minimum capacity valid for this FileSystem in bytes */
qint64 FileSystem::minCapacity() const
{
	return 8 * Capacity::unitFactor(Capacity::Byte, Capacity::MiB);
}

/** @return the maximum capacity valid for this FileSystem in bytes */
qint64 FileSystem::maxCapacity() const
{
	return Capacity::unitFactor(Capacity::Byte, Capacity::EiB);
}

/** @return the maximum label length valid for this FileSystem */
qint64 FileSystem::maxLabelLength() const
{
	return 16;
}

/** @return this FileSystem's type as printable name */
QString FileSystem::name() const
{
	return nameForType(type());
}

/** @return a pointer to a QString C array with all FileSystem names */
static const QString* typeNames()
{
	static const QString s[] =
	{
		i18nc("@item filesystem name", "unknown"),
		i18nc("@item filesystem name", "extended"),

		i18nc("@item filesystem name", "ext2"),
		i18nc("@item filesystem name", "ext3"),
		i18nc("@item filesystem name", "ext4"),
		i18nc("@item filesystem name", "linuxswap"),
		i18nc("@item filesystem name", "fat16"),
		i18nc("@item filesystem name", "fat32"),
		i18nc("@item filesystem name", "ntfs"),
		i18nc("@item filesystem name", "reiser"),
		i18nc("@item filesystem name", "reiser4"),
		i18nc("@item filesystem name", "xfs"),
		i18nc("@item filesystem name", "jfs"),
		i18nc("@item filesystem name", "hfs"),
		i18nc("@item filesystem name", "hfsplus"),
		i18nc("@item filesystem name", "ufs"),
		i18nc("@item filesystem name", "unformatted"),
		i18nc("@item filesystem name", "btrfs"),
		i18nc("@item filesystem name", "hpfs"),
		i18nc("@item filesystem name", "luks"),
		i18nc("@item filesystem name", "ocfs2"),
		i18nc("@item filesystem name", "zfs"),
		i18nc("@item filesystem name", "exfat"),
		i18nc("@item filesystem name", "nilfs2"),
		i18nc("@item filesystem name", "lvm2 pv"),
	};

	return s;
}

/** @param t the type to get the name for
	@return the printable name for the given type
*/
QString FileSystem::nameForType(FileSystem::Type t)
{
	Q_ASSERT(t >= 0);
	Q_ASSERT(t < __lastType);

	return typeNames()[t];
}

/** @param s the name to get the type for
	@return the type for the name or FileSystem::Unknown if not found
*/
FileSystem::Type FileSystem::typeForName(const QString& s)
{
	for (quint32 i = 0; i < __lastType; i++)
		if (typeNames()[i] == s)
			return static_cast<FileSystem::Type>(i);

	return Unknown;
}

/** @return a QList of all known types */
QList<FileSystem::Type> FileSystem::types()
{
	QList<FileSystem::Type> result;

	int i = Ext2; // first "real" filesystem
	while (i != __lastType)
		result.append(static_cast<FileSystem::Type>(i++));

	return result;
}

/** @return printable menu title for mounting this FileSystem */
QString FileSystem::mountTitle() const
{
	return i18nc("@title:menu", "Mount");
}

/** @return printable menu title for unmounting this FileSystem */
QString FileSystem::unmountTitle() const
{
	return i18nc("@title:menu", "Unmount");
}

/** Moves a FileSystem to a new start sector.
	@param newStartSector where the FileSystem should be moved to
*/
void FileSystem::move(qint64 newStartSector)
{
	const qint64 savedLength = length();
	setFirstSector(newStartSector);
	setLastSector(newStartSector + savedLength - 1);
}

/** Attempt to mount this FileSystem on a given mount point
	@param mountPoint the mount point to mount the FileSystem on
	@return true on success
*/
bool FileSystem::mount(const QString& mountPoint)
{
	Q_UNUSED(mountPoint);

	return false;
}

/** Attempt to unmount this FileSystem
	@param mountPoint the mount point the FileSystem is mounted on
	@return true on success
 */
bool FileSystem::unmount(const QString& mountPoint)
{
	Q_UNUSED(mountPoint);

	return false;
}

bool FileSystem::findExternal(const QString& cmdName, const QStringList& args, int expectedCode)
{
	ExternalCommand cmd(cmdName, args);
	if (!cmd.run())
		return false;

	return cmd.exitCode() == 0 || cmd.exitCode() == expectedCode;
}

FileSystem::Type FileSystem::defaultFileSystem()
{
	return static_cast<FileSystem::Type>(Config::defaultFileSystem());
}

bool FileSystem::supportToolFound() const
{
	return false;
}

FileSystem::SupportTool FileSystem::supportToolName() const
{
	return SupportTool();
}
