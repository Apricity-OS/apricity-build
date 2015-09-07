/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2003 Waldo Bastian <bastian@kde.org>
 *                2007 David Faure <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kmountpoint.h"

#include <stdlib.h>

#include <config-kmountpoint.h>

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QFileInfo>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <QDir>
#endif

#ifdef Q_OS_WIN
static Qt::CaseSensitivity cs = Qt::CaseInsensitive;
#else
static Qt::CaseSensitivity cs = Qt::CaseSensitive;
#endif

#if HAVE_VOLMGT
#include <volmgt.h>
#endif
#if HAVE_SYS_MNTTAB_H
#include <sys/mnttab.h>
#endif
#if HAVE_MNTENT_H
#include <mntent.h>
#elif HAVE_SYS_MNTENT_H
#include <sys/mntent.h>
#endif

// This is the *BSD branch
#if HAVE_SYS_MOUNT_H
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <sys/mount.h>
#endif

#if HAVE_FSTAB_H
#include <fstab.h>
#endif
#if defined(_AIX)
#include <sys/mntctl.h>
#include <sys/vmount.h>
#include <sys/vfs.h>
/* AIX does not prototype mntctl anywhere that I can find */
#ifndef mntctl
extern "C" int mntctl(int command, int size, void *buffer);
#endif
extern "C" struct vfs_ent *getvfsbytype(int vfsType);
extern "C" void endvfsent();
#endif

#if ! HAVE_GETMNTINFO
# ifdef _PATH_MOUNTED
// On some Linux, MNTTAB points to /etc/fstab !
#  undef MNTTAB
#  define MNTTAB _PATH_MOUNTED
# else
#  ifndef MNTTAB
#   ifdef MTAB_FILE
#    define MNTTAB MTAB_FILE
#   else
#    define MNTTAB "/etc/mnttab"
#   endif
#  endif
# endif
#endif

#ifdef _OS_SOLARIS_
#define FSTAB "/etc/vfstab"
#else
#define FSTAB "/etc/fstab"
#endif

class KMountPoint::Private
{
public:
    void finalizePossibleMountPoint(DetailsNeededFlags infoNeeded);
    void finalizeCurrentMountPoint(DetailsNeededFlags infoNeeded);

    QString mountedFrom;
    QString device; // Only available when the NeedRealDeviceName flag was set.
    QString mountPoint;
    QString mountType;
    QStringList mountOptions;
};

KMountPoint::KMountPoint()
    : d(new Private)
{
}

KMountPoint::~KMountPoint()
{
    delete d;
}

// There are (at least) four kind of APIs:
// setmntent + getmntent + struct mntent (linux...)
//             getmntent + struct mnttab
// mntctl                + struct vmount (AIX)
// getmntinfo + struct statfs&flags (BSD 4.4 and friends)
// getfsent + char* (BSD 4.3 and friends)

#if HAVE_SETMNTENT
#define SETMNTENT setmntent
#define ENDMNTENT endmntent
#define STRUCT_MNTENT struct mntent *
#define STRUCT_SETMNTENT FILE *
#define GETMNTENT(file, var) ((var = getmntent(file)) != 0)
#define MOUNTPOINT(var) var->mnt_dir
#define MOUNTTYPE(var) var->mnt_type
#define MOUNTOPTIONS(var) var->mnt_opts
#define FSNAME(var) var->mnt_fsname
#else
#define SETMNTENT fopen
#define ENDMNTENT fclose
#define STRUCT_MNTENT struct mnttab
#define STRUCT_SETMNTENT FILE *
#define GETMNTENT(file, var) (getmntent(file, &var) == 0)
#define MOUNTPOINT(var) var.mnt_mountp
#define MOUNTTYPE(var) var.mnt_fstype
#define MOUNTOPTIONS(var) var.mnt_mntopts
#define FSNAME(var) var.mnt_special
#endif

/**
 * When using supermount, the device name is in the options field
 * as dev=/my/device
 */
static QString devNameFromOptions(const QStringList &options)
{
    // Search options to find the device name
    for (QStringList::ConstIterator it = options.begin(); it != options.end(); ++it) {
        if ((*it).startsWith(QLatin1String("dev="))) {
            return (*it).mid(4);
        }
    }
    return QString::fromLatin1("none");
}

void KMountPoint::Private::finalizePossibleMountPoint(DetailsNeededFlags infoNeeded)
{
    if (mountType == QLatin1String("supermount")) {
        mountedFrom = devNameFromOptions(mountOptions);
    }

    if (mountedFrom.startsWith(QLatin1String("UUID="))) {
        const QString uuid = mountedFrom.mid(5);
        const QString potentialDevice = QFile::symLinkTarget(QString::fromLatin1("/dev/disk/by-uuid/") + uuid);
        if (QFile::exists(potentialDevice)) {
            mountedFrom = potentialDevice;
        }
    }
    if (mountedFrom.startsWith(QLatin1String("LABEL="))) {
        const QString label = mountedFrom.mid(6);
        const QString potentialDevice = QFile::symLinkTarget(QString::fromLatin1("/dev/disk/by-label/") + label);
        if (QFile::exists(potentialDevice)) {
            mountedFrom = potentialDevice;
        }
    }

    if (infoNeeded & NeedRealDeviceName) {
        if (mountedFrom.startsWith(QLatin1Char('/'))) {
            device = QFileInfo(mountedFrom).canonicalFilePath();
        }
    }
    // TODO: Strip trailing '/' ?
}

void KMountPoint::Private::finalizeCurrentMountPoint(DetailsNeededFlags infoNeeded)
{
    if (infoNeeded & NeedRealDeviceName) {
        if (mountedFrom.startsWith(QLatin1Char('/'))) {
            device = QFileInfo(mountedFrom).canonicalFilePath();
        }
    }
}

KMountPoint::List KMountPoint::possibleMountPoints(DetailsNeededFlags infoNeeded)
{
#ifdef Q_OS_WIN
    return KMountPoint::currentMountPoints(infoNeeded);
#endif

    KMountPoint::List result;

#if HAVE_SETMNTENT
    STRUCT_SETMNTENT fstab;
    if ((fstab = SETMNTENT(FSTAB, "r")) == 0) {
        return result;
    }

    STRUCT_MNTENT fe;
    while (GETMNTENT(fstab, fe)) {
        Ptr mp(new KMountPoint);
        mp->d->mountedFrom = QFile::decodeName(FSNAME(fe));

        mp->d->mountPoint = QFile::decodeName(MOUNTPOINT(fe));
        mp->d->mountType = QFile::decodeName(MOUNTTYPE(fe));

        //Devices using supermount have their device names in the mount options
        //instead of the device field. That's why we need to read the mount options
        if (infoNeeded & NeedMountOptions || (mp->d->mountType == QLatin1String("supermount"))) {
            QString options = QFile::decodeName(MOUNTOPTIONS(fe));
            mp->d->mountOptions = options.split(QLatin1Char(','));
        }

        mp->d->finalizePossibleMountPoint(infoNeeded);

        result.append(mp);
    }
    ENDMNTENT(fstab);
#else
    QFile f(QLatin1String(FSTAB));
    if (!f.open(QIODevice::ReadOnly)) {
        return result;
    }

    QTextStream t(&f);
    QString s;

    while (! t.atEnd()) {
        s = t.readLine().simplified();
        if (s.isEmpty() || (s[0] == QLatin1Char('#'))) {
            continue;
        }

        // not empty or commented out by '#'
        const QStringList item = s.split(QLatin1Char(' '));

#ifdef _OS_SOLARIS_
        if (item.count() < 5) {
            continue;
        }
#else
        if (item.count() < 4) {
            continue;
        }
#endif

        Ptr mp(new KMountPoint);

        int i = 0;
        mp->d->mountedFrom = item[i++];
#ifdef _OS_SOLARIS_
        //device to fsck
        i++;
#endif
        mp->d->mountPoint = item[i++];
        mp->d->mountType = item[i++];
        QString options = item[i++];

        if (infoNeeded & NeedMountOptions) {
            mp->d->mountOptions = options.split(QLatin1Char(','));
        }

        mp->d->finalizePossibleMountPoint(infoNeeded);

        result.append(mp);
    } //while

    f.close();
#endif
    return result;
}

KMountPoint::List KMountPoint::currentMountPoints(DetailsNeededFlags infoNeeded)
{
    KMountPoint::List result;

#if HAVE_GETMNTINFO

#if GETMNTINFO_USES_STATVFS
    struct statvfs *mounted;
#else
    struct statfs *mounted;
#endif

    int num_fs = getmntinfo(&mounted, MNT_NOWAIT);

    for (int i = 0; i < num_fs; i++) {
        Ptr mp(new KMountPoint);
        mp->d->mountedFrom = QFile::decodeName(mounted[i].f_mntfromname);
        mp->d->mountPoint = QFile::decodeName(mounted[i].f_mntonname);

#ifdef __osf__
        mp->d->mountType = QFile::decodeName(mnt_names[mounted[i].f_type]);
#else
        mp->d->mountType = QFile::decodeName(mounted[i].f_fstypename);
#endif

        if (infoNeeded & NeedMountOptions) {
            struct fstab *ft = getfsfile(mounted[i].f_mntonname);
            if (ft != 0) {
                QString options = QFile::decodeName(ft->fs_mntops);
                mp->d->mountOptions = options.split(QLatin1Char(','));
            } else {
                // TODO: get mount options if not mounted via fstab, see mounted[i].f_flags
            }
        }

        mp->d->finalizeCurrentMountPoint(infoNeeded);
        // TODO: Strip trailing '/' ?
        result.append(mp);
    }

#elif defined(_AIX)

    struct vmount *mntctl_buffer;
    struct vmount *vm;
    char *mountedfrom;
    char *mountedto;
    int fsname_len, num;
    int buf_sz = 4096;

    mntctl_buffer = (struct vmount *)malloc(buf_sz);
    num = mntctl(MCTL_QUERY, buf_sz, mntctl_buffer);
    if (num == 0) {
        buf_sz = *(int *)mntctl_buffer;
        free(mntctl_buffer);
        mntctl_buffer = (struct vmount *)malloc(buf_sz);
        num = mntctl(MCTL_QUERY, buf_sz, mntctl_buffer);
    }

    if (num > 0) {
        /* iterate through items in the vmount structure: */
        vm = (struct vmount *)mntctl_buffer;
        for (; num > 0; --num) {
            /* get the name of the mounted file systems: */
            fsname_len = vmt2datasize(vm, VMT_STUB);
            mountedto     = (char *)malloc(fsname_len + 1);
            mountedto[fsname_len] = '\0';
            strncpy(mountedto, (char *)vmt2dataptr(vm, VMT_STUB), fsname_len);

            fsname_len = vmt2datasize(vm, VMT_OBJECT);
            mountedfrom     = (char *)malloc(fsname_len + 1);
            mountedfrom[fsname_len] = '\0';
            strncpy(mountedfrom, (char *)vmt2dataptr(vm, VMT_OBJECT), fsname_len);

            /* Look up the string for the file system type,
                 * as listed in /etc/vfs.
                 * ex.: nfs,jfs,afs,cdrfs,sfs,cachefs,nfs3,autofs
                 */
            struct vfs_ent *ent = getvfsbytype(vm->vmt_gfstype);

            KMountPoint *mp = new KMountPoint;
            mp->d->mountedFrom = QFile::decodeName(mountedfrom);
            mp->d->mountPoint = QFile::decodeName(mountedto);
            mp->d->mountType = QFile::decodeName(ent->vfsent_name);

            free(mountedfrom);
            free(mountedto);

            if (infoNeeded & NeedMountOptions) {
                // TODO
            }

            mp->d->finalizeCurrentMountPoint(infoNeeded);
            result.append(mp);

            /* goto the next vmount structure: */
            vm = (struct vmount *)((char *)vm + vm->vmt_length);
        }

        endvfsent();
    }

    free(mntctl_buffer);
#elif defined(Q_OS_WIN) && !defined(_WIN32_WCE)
    //nothing fancy with infoNeeded but it gets the job done
    DWORD bits = GetLogicalDrives();
    if (!bits) {
        return result;
    }

    for (int i = 0; i < 26; i++) {
        if (bits & (1 << i)) {
            Ptr mp(new KMountPoint);
            mp->d->mountPoint = QString(QLatin1Char('A' + i) + QLatin1String(":/"));
            result.append(mp);
        }
    }

#elif defined(_WIN32_WCE)
    Ptr mp(new KMountPoint);
    mp->d->mountPoint = QString("/");
    result.append(mp);

#else
    STRUCT_SETMNTENT mnttab;
    if ((mnttab = SETMNTENT(MNTTAB, "r")) == 0) {
        return result;
    }

    STRUCT_MNTENT fe;
    while (GETMNTENT(mnttab, fe)) {
        Ptr mp(new KMountPoint);
        mp->d->mountedFrom = QFile::decodeName(FSNAME(fe));

        mp->d->mountPoint = QFile::decodeName(MOUNTPOINT(fe));
        mp->d->mountType = QFile::decodeName(MOUNTTYPE(fe));

        //Devices using supermount have their device names in the mount options
        //instead of the device field. That's why we need to read the mount options
        if (infoNeeded & NeedMountOptions || (mp->d->mountType == QLatin1String("supermount"))) {
            QString options = QFile::decodeName(MOUNTOPTIONS(fe));
            mp->d->mountOptions = options.split(QLatin1Char(','));
        }
        mp->d->finalizeCurrentMountPoint(infoNeeded);

        result.append(mp);
    }
    ENDMNTENT(mnttab);
#endif
    return result;
}

QString KMountPoint::mountedFrom() const
{
    return d->mountedFrom;
}

QString KMountPoint::realDeviceName() const
{
    return d->device;
}

QString KMountPoint::mountPoint() const
{
    return d->mountPoint;
}

QString KMountPoint::mountType() const
{
    return d->mountType;
}

QStringList KMountPoint::mountOptions() const
{
    return d->mountOptions;
}

KMountPoint::List::List()
    : QList<Ptr>()
{
}

static bool pathsAreParentAndChildOrEqual(const QString &parent, const QString &child)
{
    const QLatin1Char slash('/');
    if (child.startsWith(parent, cs)) {
        // Check if either
        // (a) both paths are equal, or
        // (b) parent ends with '/', or
        // (c) the first character of child that is not shared with parent is '/'.
        //     Note that child is guaranteed to be longer than parent if (a) is false.
        //
        // This prevents that we incorrectly consider "/books" a child of "/book".
        return parent.compare(child, cs) == 0 || parent.endsWith(slash) || child.at(parent.length()) == slash;
    } else {
        // Note that "/books" is a child of "/books/".
        return parent.endsWith(slash) && (parent.length() == child.length() + 1) && parent.startsWith(child, cs);
    }
}

KMountPoint::Ptr KMountPoint::List::findByPath(const QString &path) const
{
#ifndef Q_OS_WIN
    /* If the path contains symlinks, get the real name */
    QFileInfo fileinfo(path);
    const QString realname = fileinfo.exists()
                             ? fileinfo.canonicalFilePath()
                             : fileinfo.absolutePath(); //canonicalFilePath won't work unless file exists
#else
    const QString realname = QDir::fromNativeSeparators(QDir(path).absolutePath());
#endif

    int max = 0;
    KMountPoint::Ptr result;
    for (const_iterator it = begin(); it != end(); ++it) {
        const QString mountpoint = (*it)->d->mountPoint;
        const int length = mountpoint.length();
        if (length > max && pathsAreParentAndChildOrEqual(mountpoint, realname)) {
            max = length;
            result = *it;
            // keep iterating to check for a better match (bigger max)
        }
    }
    return result;
}

KMountPoint::Ptr KMountPoint::List::findByDevice(const QString &device) const
{
    const QString realDevice = QFileInfo(device).canonicalFilePath();
    if (realDevice.isEmpty()) { // d->device can be empty in the loop below, don't match empty with it
        return Ptr();
    }
    for (const_iterator it = begin(); it != end(); ++it) {
        if (realDevice.compare((*it)->d->device, cs) == 0 ||
                realDevice.compare((*it)->d->mountedFrom, cs) == 0) {
            return *it;
        }
    }
    return Ptr();
}

bool KMountPoint::probablySlow() const
{
    bool nfs = d->mountType == QLatin1String("nfs");
    bool cifs = d->mountType == QLatin1String("cifs");
    bool autofs = d->mountType == QLatin1String("autofs") || d->mountType == QLatin1String("subfs");
    if (nfs || autofs || cifs) {
        return true;
    }
    return false;
}

bool KMountPoint::testFileSystemFlag(FileSystemFlag flag) const
{
    const bool isMsDos = (d->mountType == QLatin1String("msdos") || d->mountType == QLatin1String("fat") || d->mountType == QLatin1String("vfat"));
    const bool isNtfs = d->mountType.contains(QLatin1String("fuse.ntfs")) || d->mountType.contains(QLatin1String("fuseblk.ntfs"))
                        // fuseblk could really be anything. But its most common use is for NTFS mounts, these days.
                        || d->mountType == QLatin1String("fuseblk");
    const bool isSmb = d->mountType == QLatin1String("cifs") || d->mountType == QLatin1String("smbfs");

    switch (flag)  {
    case SupportsChmod:
    case SupportsChown:
    case SupportsUTime:
    case SupportsSymlinks:
        return !isMsDos && !isNtfs && !isSmb; // it's amazing the number of things Microsoft filesystems don't support :)
    case CaseInsensitive:
        return isMsDos;
    }
    return false;
}

