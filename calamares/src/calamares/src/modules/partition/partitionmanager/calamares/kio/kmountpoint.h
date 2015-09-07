/*
   This file is part of the KDE libraries
   Copyright (c) 2003 Waldo Bastian <bastian@kde.org>
                 2007 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KMOUNTPOINT_H
#define KMOUNTPOINT_H

#include "kiocore_export.h"
#include <QExplicitlySharedDataPointer>

#include <QtCore/QStringList>

/**
 * The KMountPoint class provides information about mounted and unmounted disks.
 * It provides a system independent interface to fstab.
 *
 * @author Waldo Bastian <bastian@kde.org>
 */
class KIOCORE_EXPORT KMountPoint : public QSharedData
{
public:
    typedef QExplicitlySharedDataPointer<KMountPoint> Ptr;
    /**
     * List of mount points.
     */
    class KIOCORE_EXPORT List : public QList<Ptr>
    {
    public:
        List();
        /**
         * Find the mountpoint on which resides @p path
         * For instance if /home is a separate partition, findByPath("/home/user/blah")
         * will return /home
         * @param path the path to check
         * @return the mount point of the given file
         */
        Ptr findByPath(const QString &path) const;

        /**
         * Returns the mount point associated with @p device,
         * i.e. the one where mountedFrom() == @p device
         * (after symlink resolution).
         * @return the mountpoint, or 0 if this device doesn't exist or isn't mounted
         */
        Ptr findByDevice(const QString &device) const;
    };
public:
    /**
     * Flags that specify which additional details should be fetched for each mountpoint.
     * BasicInfoNeeded: only the basic details: mountedFrom, mountPoint, mountType.
     * NeedMountOptions: also fetch the options used when mounting, see mountOptions.
     * NeedRealDeviceName: also fetch the device name (with symlinks resolved), see realDeviceName.
     */
    enum DetailsNeededFlag { BasicInfoNeeded = 0, NeedMountOptions = 1, NeedRealDeviceName = 2 };
    Q_DECLARE_FLAGS(DetailsNeededFlags, DetailsNeededFlag)

    /**
     * This function gives a list of all possible mountpoints. (fstab)
     * @param infoNeeded Flags that specify which additional information
     * should be fetched.
     */
    static List possibleMountPoints(DetailsNeededFlags infoNeeded = BasicInfoNeeded);

    /**
     * This function gives a list of all currently used mountpoints. (mtab)
     * @param infoNeeded Flags that specify which additional information
     * should be fetched.
     */
    static List currentMountPoints(DetailsNeededFlags infoNeeded = BasicInfoNeeded);

    /**
     * Where this filesystem gets mounted from.
     * This can refer to a device, a remote server or something else.
     */
    QString mountedFrom() const;

    /**
     * Canonical name of the device where the filesystem got mounted from.
     * (Or empty, if not a device)
     * Only available when the NeedRealDeviceName flag was set.
     */
    QString realDeviceName() const;

    /**
     * Path where the filesystem is mounted or can be mounted.
     */
    QString mountPoint() const;

    /**
     * Type of filesystem
     */
    QString mountType() const;

    /**
     * Options used to mount the filesystem.
     * Only available when the NeedMountOptions flag was set.
     */
    QStringList mountOptions() const;

    /**
     * Checks if the filesystem that is probably slow (network mounts).
     * @return true if the filesystem is probably slow
     */
    bool probablySlow() const;

    enum FileSystemFlag { SupportsChmod, SupportsChown, SupportsUTime,
                          SupportsSymlinks, CaseInsensitive
                        };
    /**
     * Checks the capabilities of the filesystem.
     * @param flag the flag to check
     * @return true if the filesystem has that flag, false if not
     *
     * The availables flags are:
     * @li SupportsChmod: returns true if the filesystem supports chmod
     * (e.g. msdos filesystems return false)
     * @li SupportsChown: returns true if the filesystem supports chown
     * (e.g. msdos filesystems return false)
     * @li SupportsUtime: returns true if the filesystems supports utime
     * (e.g. msdos filesystems return false)
     * @li SupportsSymlinks: returns true if the filesystems supports symlinks
     * (e.g. msdos filesystems return false)
     * @li CaseInsensitive: returns true if the filesystem treats
     * "foo" and "FOO" as being the same file (true for msdos systems)
     *
     */
    bool testFileSystemFlag(FileSystemFlag flag) const;

    /**
     * Destructor
     */
    ~KMountPoint();

private:
    /**
     * Constructor
     */
    KMountPoint();

    class Private;
    Private *const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KMountPoint::DetailsNeededFlags)

#endif // KMOUNTPOINT_H

