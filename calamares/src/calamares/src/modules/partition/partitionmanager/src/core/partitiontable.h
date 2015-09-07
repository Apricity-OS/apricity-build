/***************************************************************************
 *   Copyright (C) 2008,2010 by Volker Lanz <vl@fidra.de>                  *
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

#if !defined(PARTITIONTABLE__H)

#define PARTITIONTABLE__H

#include "util/libpartitionmanagerexport.h"

#include "core/partitionnode.h"
#include "core/partitionrole.h"

#include <QList>
#include <qglobal.h>

class Device;
class Partition;
class CoreBackend;

class QTextStream;

/** The partition table (a.k.a Disk Label)

	PartitionTable represents a partition table (or disk label).

	PartitionTable has child nodes that represent Partitions.

	@author Volker Lanz <vl@fidra.de>
*/
class LIBPARTITIONMANAGERPRIVATE_EXPORT PartitionTable : public PartitionNode
{
	Q_DISABLE_COPY(PartitionTable)

	friend class CoreBackend;
	friend QTextStream& operator<<(QTextStream& stream, const PartitionTable& ptable);

	public:
		enum TableType
		{
			unknownTableType = -1,

			aix,
			bsd,
			dasd,
			msdos,
			msdos_sectorbased,
			dvh,
			gpt,
			loop,
			mac,
			pc98,
			amiga,
			sun
		};

		/** Partition flags */
		enum Flag
		{
			FlagNone = 0,
			FlagBoot = 1,
			FlagRoot = 2,
			FlagSwap = 4,
			FlagHidden = 8,
			FlagRaid = 16,
			FlagLvm = 32,
			FlagLba = 64,
			FlagHpService = 128,
			FlagPalo = 256,
			FlagPrep = 512,
			FlagMsftReserved = 1024
		};

		Q_DECLARE_FLAGS(Flags, Flag)

	public:
		PartitionTable(TableType type, qint64 first_usable, qint64 last_usable);
		~PartitionTable();

	public:
		PartitionNode* parent() { return NULL; } /**< @return always NULL for PartitionTable */
		const PartitionNode* parent() const { return NULL; } /**< @return always NULL for PartitionTable */

		bool isRoot() const { return true; } /**< @return always true for PartitionTable */
		bool isReadOnly() const { return tableTypeIsReadOnly(type()); } /**< @return true if the PartitionTable is read only */

		Partitions& children() { return m_Children; } /**< @return the children in this PartitionTable */
		const Partitions& children() const { return m_Children; } /**< @return the children in this PartitionTable */

		void setType(const Device& d, TableType t);

		void append(Partition* partition);

		qint64 freeSectorsBefore(const Partition& p) const;
		qint64 freeSectorsAfter(const Partition& p) const;

		bool hasExtended() const;
		Partition* extended() const;

		PartitionRole::Roles childRoles(const Partition& p) const;

		int numPrimaries() const;
		int maxPrimaries() const { return m_MaxPrimaries; } /**< @return max number of primary partitions this PartitionTable can handle */

		PartitionTable::TableType type() const { return m_Type; } /**< @return the PartitionTable's type */
		const QString typeName() const { return tableTypeToName(type()); } /**< @return the name of this PartitionTable type */

		qint64 firstUsable() const { return m_FirstUsable; }
		qint64 lastUsable() const { return m_LastUsable; }

		void updateUnallocated(const Device& d);
		void insertUnallocated(const Device& d, PartitionNode* p, qint64 start) const;

		bool isSectorBased(const Device& d) const;

		static QList<Flag> flagList();
		static QString flagName(Flag f);
		static QStringList flagNames(Flags f);

		static bool getUnallocatedRange(const Device& device, PartitionNode& parent, qint64& start, qint64& end);

		static void removeUnallocated(PartitionNode* p);
		void removeUnallocated();

		static qint64 defaultFirstUsable(const Device& d, TableType t);
		static qint64 defaultLastUsable(const Device& d, TableType t);

		static PartitionTable::TableType nameToTableType(const QString& n);
		static QString tableTypeToName(TableType l);
		static qint64 maxPrimariesForTableType(TableType l);
		static bool tableTypeSupportsExtended(TableType l);
		static bool tableTypeIsReadOnly(TableType l);

	protected:
		void setMaxPrimaries(qint32 n) { m_MaxPrimaries = n; }
		void setFirstUsableSector(qint64 s) { m_FirstUsable = s; }
		void setLastUsableSector(qint64 s) { m_LastUsable = s; }

	private:
		Partitions m_Children;
		qint32 m_MaxPrimaries;
		TableType m_Type;
		qint64 m_FirstUsable;
		qint64 m_LastUsable;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PartitionTable::Flags)

QTextStream& operator<<(QTextStream& stream, const PartitionTable& ptable);

#endif

