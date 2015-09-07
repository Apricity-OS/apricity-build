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

#if !defined(PARTITIONNODE__H)

#define PARTITIONNODE__H

#include <QObject>
#include <QList>
#include <qglobal.h>

class Partition;
class PartitionRole;

/** A node in the tree of partitions.

	The root in this tree is the PartitionTable. The primaries are the child nodes; extended partitions again
	have child nodes.

	@see Device, PartitionTable, Partition
	@author Volker Lanz <vl@fidra.de>
*/
class PartitionNode : public QObject
{
	Q_OBJECT

	public:
		typedef QList<Partition*> Partitions;

	protected:
		PartitionNode() {}
		virtual ~PartitionNode() {}

	public:
		virtual bool insert(Partition* partNew);
		
		virtual Partition* predecessor(Partition& p);
		virtual const Partition* predecessor(const Partition& p) const;

		virtual Partition* successor(Partition& p);
		virtual const Partition* successor(const Partition& p) const;

		virtual bool remove(Partition* p);
		virtual Partition* findPartitionBySector(qint64 s, const PartitionRole& role);
		virtual const Partition* findPartitionBySector(qint64 s, const PartitionRole& role) const;
		virtual void reparent(Partition& p);

		virtual Partitions& children() = 0;
		virtual PartitionNode* parent() = 0;
		virtual bool isRoot() const = 0;
		virtual const PartitionNode* parent() const = 0;
		virtual const Partitions& children() const = 0;
		virtual void append(Partition* p) = 0;
		virtual qint32 highestMountedChild() const;
		virtual bool isChildMounted() const;

	protected:
		virtual void clearChildren();
};

#endif
