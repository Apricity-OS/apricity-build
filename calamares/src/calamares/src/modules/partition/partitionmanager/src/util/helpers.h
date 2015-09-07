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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#if !defined(HELPERS__H)

#define HELPERS__H

#include "util/libpartitionmanagerexport.h"

#include "fs/filesystem.h"

class KAboutData;
class QString;
class QIcon;
class QPoint;
class QTreeWidget;

namespace Solid
{
	class Device;
}

LIBPARTITIONMANAGERPRIVATE_EXPORT void registerMetaTypes();
LIBPARTITIONMANAGERPRIVATE_EXPORT bool checkPermissions();

LIBPARTITIONMANAGERPRIVATE_EXPORT KAboutData* createPartitionManagerAboutData();

LIBPARTITIONMANAGERPRIVATE_EXPORT bool caseInsensitiveLessThan(const QString& s1, const QString& s2);
LIBPARTITIONMANAGERPRIVATE_EXPORT bool naturalLessThan(const QString& s1, const QString& s2);

LIBPARTITIONMANAGERPRIVATE_EXPORT QIcon createFileSystemColor(FileSystem::Type type, quint32 size);

LIBPARTITIONMANAGERPRIVATE_EXPORT void showColumnsContextMenu(const QPoint& p, QTreeWidget& tree);

LIBPARTITIONMANAGERPRIVATE_EXPORT bool loadBackend();

LIBPARTITIONMANAGERPRIVATE_EXPORT QList<Solid::Device> getSolidDeviceList();

LIBPARTITIONMANAGERPRIVATE_EXPORT bool checkAccessibleDevices();

#endif
