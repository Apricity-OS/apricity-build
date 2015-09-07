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

#if !defined(PARTITIONROLE__H)

#define PARTITIONROLE__H

#include "util/libpartitionmanagerexport.h"

#include <qglobal.h>

class QString;

/** A Partition's role.

	Each Partition has a PartitionRole: It can be primary, extended, logical or represent unallocated space on the Device.

	@author Volker Lanz <vl@fidra.de>
*/
class LIBPARTITIONMANAGERPRIVATE_EXPORT PartitionRole
{
	public:
		/** A Partition's role: What kind of Partition is it? */
		enum Role
		{
			None = 0,			/**< None at all */
			Primary = 1,		/**< Primary */
			Extended = 2,		/**< Extended */
			Logical = 4,		/**< Logical inside an extended */
			Unallocated = 8,	/**< No real Partition, just unallocated space */

			Any = 255			/**< In case we're looking for a Partition with a PartitionRole, any will do */
		};

 	Q_DECLARE_FLAGS(Roles, Role)

	public:
		explicit PartitionRole(Roles r) : m_Roles(r) {} /**< Creates a new PartitionRole object */
		Roles roles() const { return m_Roles; } /**< @return the roles as bitfield */
		bool has(Role r) const { return roles() & r; } /**< @param r the role to check @return true if the role is set */

		bool operator==(const PartitionRole& other) const { return m_Roles == other.m_Roles; } /**< @param other object to compare with @return true if the same */
		bool operator!=(const PartitionRole& other) const { return !operator==(other); }  /**< @param other object to compare with @return true if not the same */

		QString toString() const;

	private:
		Roles m_Roles;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PartitionRole::Roles)

#endif
