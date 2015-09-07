/***************************************************************************
 *   Copyright (C) 2008, 2009, 2010 by Volker Lanz <vl@fidra.de>           *
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

// This version of helpers.cpp contains only the functions we need for
// libcalapm. This trims dependencies as the original helpers.cpp brings in
// KCoreAddons and KWidgetsAddons

#include <util/helpers.h>

#include <Solid/Device>

QList<Solid::Device> getSolidDeviceList()
{
#ifdef ENABLE_UDISKS2
    QString predicate = QStringLiteral("StorageVolume.usage == 'PartitionTable'");

#else
    QString predicate = "[ [ [ StorageDrive.driveType == 'HardDisk' OR StorageDrive.driveType == 'CompactFlash'] OR "
                "[ StorageDrive.driveType == 'MemoryStick' OR StorageDrive.driveType == 'SmartMedia'] ] OR "
                "[ StorageDrive.driveType == 'SdMmc' OR StorageDrive.driveType == 'Xd'] ]";
#endif
    return Solid::Device::listFromQuery(predicate);
}
