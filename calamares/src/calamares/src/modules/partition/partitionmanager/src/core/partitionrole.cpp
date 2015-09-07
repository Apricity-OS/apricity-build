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

#include "core/partitionrole.h"

#include <QString>
#include <KLocalizedString>

/** @return the role as string */
QString PartitionRole::toString() const
{
	if (roles() & Unallocated)
		return i18nc("@item partition role", "unallocated");

	if (roles() & Logical)
		return i18nc("@item partition role", "logical");

	if (roles() & Extended)
		return i18nc("@item partition role", "extended");

	if (roles() & Primary)
		return i18nc("@item partition role", "primary");

	return i18nc("@item partition role", "none");
}
