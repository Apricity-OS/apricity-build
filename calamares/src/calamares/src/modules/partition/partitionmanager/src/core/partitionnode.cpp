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

#include "core/partitionnode.h"

#include "core/partition.h"
#include "core/partitionrole.h"

#include "fs/filesystem.h"

/** Tries to find the predecessor for a Partition.
	@param p the Partition to find a predecessor for
	@return pointer to the predecessor or NULL if none was found
*/
Partition* PartitionNode::predecessor(Partition& p)
{
	Q_ASSERT(p.parent());

	Partitions& plist = p.parent()->isRoot() == false ? p.parent()->children() : children();

	for (int idx = 1; idx < plist.size(); idx++)
		if (plist[idx] == &p)
			return plist[idx - 1];

	return NULL;
}

/**
	@overload
*/
const Partition* PartitionNode::predecessor(const Partition& p) const
{
	Q_ASSERT(p.parent());

	const Partitions& plist = p.parent()->isRoot() == false ? p.parent()->children() : children();

	for (int idx = 1; idx < plist.size(); idx++)
		if (plist[idx] == &p)
			return plist[idx - 1];

	return NULL;
}

/** Tries to find the successor for a Partition.
	@param p the Partition to find a successor for
	@return pointer to the successor or NULL if none was found
 */
Partition* PartitionNode::successor(Partition& p)
{
	Q_ASSERT(p.parent());

	Partitions& plist = p.parent()->isRoot() == false ? p.parent()->children() : children();

	for (int idx = plist.size() - 2; idx >= 0; idx--)
		if (plist[idx] == &p)
			return plist[idx + 1];

	return NULL;
}

/**
	@overload
*/
const Partition* PartitionNode::successor(const Partition& p) const
{
	Q_ASSERT(p.parent());

	const Partitions& plist = p.parent()->isRoot() == false ? p.parent()->children() : children();

	for (int idx = plist.size() - 2; idx >= 0; idx--)
		if (plist[idx] == &p)
			return plist[idx + 1];

	return NULL;
}

/** Inserts a Partition into a PartitionNode's children
	@param p pointer to the Partition to insert. May be NULL.
	@return true on success
*/
bool PartitionNode::insert(Partition* p)
{
	if (p == NULL)
		return false;

	for (int idx = 0; idx < children().size(); idx++)
	{
		if (children()[idx]->firstSector() > p->firstSector())
		{
			children().insert(idx, p);
			return true;
		}
	}

	children().insert(children().size(), p);

	return true;
}

/** Removes a Partition from the PartitionNode's children.
	@param p pointer to the Partition to remove. May be NULL.
	@return true on success.
*/
bool PartitionNode::remove(Partition* p)
{
	if (p == NULL)
		return false;

	if (children().removeOne(p))
		return true;

	return false;
}

/** Deletes all children */
void PartitionNode::clearChildren()
{
	qDeleteAll(children());
	children().clear();
}

/** Finds a Partition by sector.
	@param s the sector the Partition is at
	@param role the PartitionRole the Partition is supposed to have
	@return pointer to the Partition found or NULL if none was found
*/
Partition* PartitionNode::findPartitionBySector(qint64 s, const PartitionRole& role)
{
	foreach (Partition* p, children())
	{
		// (women and) children first. ;-)
		foreach (Partition* child, p->children())
			if ((child->roles().roles() & role.roles()) && s >= child->firstSector() && s <= child->lastSector())
				return child;

		if ((p->roles().roles() & role.roles()) && s >= p->firstSector() && s <= p->lastSector())
			return p;
	}

	return NULL;
}

/**
	@overload
*/
const Partition* PartitionNode::findPartitionBySector(qint64 s, const PartitionRole& role) const
{
	foreach (const Partition* p, children())
	{
		foreach (const Partition* child, p->children())
			if ((child->roles().roles() & role.roles()) && s >= child->firstSector() && s <= child->lastSector())
				return child;

		if ((p->roles().roles() & role.roles()) && s >= p->firstSector() && s <= p->lastSector())
			return p;
	}

	return NULL;
}

/** Reparents a Partition to this PartitionNode
	@param p the Partition to reparent
*/
void PartitionNode::reparent(Partition& p)
{
	p.setParent(this);

	if (!isRoot())
		p.setRoles(PartitionRole(PartitionRole::Logical));
	else if (!p.roles().has(PartitionRole::Extended))
		p.setRoles(PartitionRole(PartitionRole::Primary));
	else
		p.setRoles(PartitionRole(PartitionRole::Extended));
}

/** @return the number of the highest mounted child, e.g. 7 if /dev/sdd7 is a child of this PartitionNode and mounted and /dev/sdd8 and /dev/sdd9 and so on aren't
*/
qint32 PartitionNode::highestMountedChild() const
{
	qint32 result = -1;

	foreach (const Partition* p, children())
		if (p->number() > result && p->isMounted())
			result = p->number();

	return result;
}

/** @return true if any of the partition's children are mounted */
bool PartitionNode::isChildMounted() const
{
	foreach (const Partition* child, children())
		if (child->isMounted() || (child->hasChildren() && child->isChildMounted()))
			return true;

	return false;
}
