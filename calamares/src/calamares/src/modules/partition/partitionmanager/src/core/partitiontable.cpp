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

/** @file
*/

#include "core/partitiontable.h"
#include "core/partition.h"
#include "core/device.h"
#include "core/partitionalignment.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"

#include "util/globallog.h"

#include <KLocalizedString>

#include <QDebug>
#include <QFile>
#include <QTextStream>

#include <config.h>

/** Creates a new PartitionTable object with type MSDOS
	@param type name of the PartitionTable type (e.g. "msdos" or "gpt")
*/
PartitionTable::PartitionTable(TableType type, qint64 first_usable, qint64 last_usable) :
	PartitionNode(),
	m_Children(),
	m_MaxPrimaries(maxPrimariesForTableType(type)),
	m_Type(type),
	m_FirstUsable(first_usable),
	m_LastUsable(last_usable)
{
}

/** Destroys a PartitionTable object, destroying all children */
PartitionTable::~PartitionTable()
{
	clearChildren();
}

/** Gets the number of free sectors before a given child Partition in this PartitionTable.

	@param p the Partition for which to get the free sectors before
	@returns the number of free sectors before the Partition
*/
qint64 PartitionTable::freeSectorsBefore(const Partition& p) const
{
	const Partition* pred = predecessor(p);

	// due to the space required for extended boot records the
	// below is NOT the same as pred->length()
	if (pred && pred->roles().has(PartitionRole::Unallocated))
		return p.firstSector() - pred->firstSector();

	return 0;
}

/** Gets the number of free sectors after a given child Partition in this PartitionTable.

	@param p the Partition for which to get the free sectors after
	@returns the number of free sectors after the Partition
*/
qint64 PartitionTable::freeSectorsAfter(const Partition& p) const
{
	const Partition* succ = successor(p);

	// due to the space required for extended boot records the
	// below is NOT the same as succ->length()
	if (succ && succ->roles().has(PartitionRole::Unallocated))
		return succ->lastSector() - p.lastSector();

	return 0;
}

/** @return true if the PartitionTable has an extended Partition */
bool PartitionTable::hasExtended() const
{
	for (int i = 0; i < children().size(); i++)
		if (children()[i]->roles().has(PartitionRole::Extended))
			return true;

	return false;
}

/** @return pointer to the PartitionTable's extended Partition or NULL if none exists */
Partition* PartitionTable::extended() const
{
	for (int i = 0; i < children().size(); i++)
		if (children()[i]->roles().has(PartitionRole::Extended))
			return children()[i];

	return NULL;
}

/** Gets valid PartitionRoles for a Partition
	@param p the Partition
	@return valid roles for the given Partition
*/
PartitionRole::Roles PartitionTable::childRoles(const Partition& p) const
{
	Q_ASSERT(p.parent());

	PartitionRole::Roles r = p.parent()->isRoot() ? PartitionRole::Primary : PartitionRole::Logical;

	if (r == PartitionRole::Primary && hasExtended() == false && tableTypeSupportsExtended(type()))
		r |= PartitionRole::Extended;

	return r;
}

/** @return the number of primaries in this PartitionTable */
int PartitionTable::numPrimaries() const
{
	int result = 0;

	foreach(const Partition* p, children())
		if (p->roles().has(PartitionRole::Primary) || p->roles().has(PartitionRole::Extended))
			result++;

	return result;
}

/** Appends a Partition to this PartitionTable
	@param partition pointer of the partition to append. Must not be NULL.
*/
void PartitionTable::append(Partition* partition)
{
	children().append(partition);
}

/** @param f the flag to get the name for
	@returns the flags name or an empty QString if the flag is not known
*/
QString PartitionTable::flagName(Flag f)
{
	switch(f)
	{
		case PartitionTable::FlagBoot: return i18nc("@item partition flag", "boot");
		case PartitionTable::FlagRoot: return i18nc("@item partition flag", "root");
		case PartitionTable::FlagSwap: return i18nc("@item partition flag", "swap");
		case PartitionTable::FlagHidden: return i18nc("@item partition flag", "hidden");
		case PartitionTable::FlagRaid: return i18nc("@item partition flag", "raid");
		case PartitionTable::FlagLvm: return i18nc("@item partition flag", "lvm");
		case PartitionTable::FlagLba: return i18nc("@item partition flag", "lba");
		case PartitionTable::FlagHpService: return i18nc("@item partition flag", "hpservice");
		case PartitionTable::FlagPalo: return i18nc("@item partition flag", "palo");
		case PartitionTable::FlagPrep: return i18nc("@item partition flag", "prep");
		case PartitionTable::FlagMsftReserved: return i18nc("@item partition flag", "msft-reserved");

		default:
			break;
	}

	return QString();
}

/** @return list of all flags */
QList<PartitionTable::Flag> PartitionTable::flagList()
{
	QList<PartitionTable::Flag> rval;

	rval.append(PartitionTable::FlagBoot);
	rval.append(PartitionTable::FlagRoot);
	rval.append(PartitionTable::FlagSwap);
	rval.append(PartitionTable::FlagHidden);
	rval.append(PartitionTable::FlagRaid);
	rval.append(PartitionTable::FlagLvm);
	rval.append(PartitionTable::FlagLba);
	rval.append(PartitionTable::FlagHpService);
	rval.append(PartitionTable::FlagPalo);
	rval.append(PartitionTable::FlagPrep);
	rval.append(PartitionTable::FlagMsftReserved);

	return rval;
}

/** @param flags the flags to get the names for
	@returns QStringList of the flags' names
*/
QStringList PartitionTable::flagNames(Flags flags)
{
	QStringList rval;

	int f = 1;

	QString s;
	while(!(s = flagName(static_cast<PartitionTable::Flag>(f))).isEmpty())
	{
		if (flags & f)
			rval.append(s);

		f <<= 1;
	}

	return rval;
}

bool PartitionTable::getUnallocatedRange(const Device& device, PartitionNode& parent, qint64& start, qint64& end)
{
	if (!parent.isRoot())
	{
		Partition* extended = dynamic_cast<Partition*>(&parent);

		if (extended == NULL)
		{
			qWarning() << "extended is null. start: " << start << ", end: " << end << ", device: " << device.deviceNode();
			return false;
		}

		// Leave a track (cylinder aligned) or sector alignment sectors (sector based) free at the
		// start for a new partition's metadata
		start += device.partitionTable()->type() == PartitionTable::msdos ? device.sectorsPerTrack() : PartitionAlignment::sectorAlignment(device);

		// .. and also at the end for the metadata for a partition to follow us, if we're not
		// at the end of the extended partition
		if (end < extended->lastSector())
			end -= device.partitionTable()->type() == PartitionTable::msdos ? device.sectorsPerTrack() : PartitionAlignment::sectorAlignment(device);
	}

	return end - start + 1 >= PartitionAlignment::sectorAlignment(device);
}

/** Creates a new unallocated Partition on the given Device.
	@param device the Device to create the new Partition on
	@param parent the parent PartitionNode for the new Partition
	@param start the new Partition's start sector
	@param end the new Partition's end sector
	@return pointer to the newly created Partition object or NULL if the Partition could not be created
*/
Partition* createUnallocated(const Device& device, PartitionNode& parent, qint64 start, qint64 end)
{
	PartitionRole::Roles r = PartitionRole::Unallocated;

	if (!parent.isRoot())
		r |= PartitionRole::Logical;

	if (!PartitionTable::getUnallocatedRange(device, parent, start, end))
		return NULL;

	return new Partition(&parent, device, PartitionRole(r), FileSystemFactory::create(FileSystem::Unknown, start, end), start, end, QString());
}

/** Removes all unallocated children from a PartitionNode
	@param p pointer to the parent to remove unallocated children from
*/
void PartitionTable::removeUnallocated(PartitionNode* p)
{
	Q_ASSERT(p != NULL);

	qint32 i = 0;

	while (i < p->children().size())
	{
		Partition* child = p->children()[i];

		if (child->roles().has(PartitionRole::Unallocated))
		{
			p->remove(child);
			delete child;
			continue;
		}

		if (child->roles().has(PartitionRole::Extended))
			removeUnallocated(child);

		i++;
	}
}

/**
	@overload
*/
void PartitionTable::removeUnallocated()
{
	removeUnallocated(this);
}

/** Inserts unallocated children for a Device's PartitionTable with the given parent.

	This method inserts unallocated Partitions for a parent, usually the Device this
	PartitionTable is on. It will also insert unallocated Partitions in any extended
	Partitions it finds.

	@warning This method assumes that no unallocated Partitions exist when it is called.

	@param d the Device this PartitionTable and @p p are on
	@param p the parent PartitionNode (may be this or an extended Partition)
	@param start the first sector to begin looking for free space
*/
void PartitionTable::insertUnallocated(const Device& d, PartitionNode* p, qint64 start) const
{
	Q_ASSERT(p != NULL);

	qint64 lastEnd = start;

	foreach (Partition* child, p->children())
	{
		p->insert(createUnallocated(d, *p, lastEnd, child->firstSector() - 1));

		if (child->roles().has(PartitionRole::Extended))
			insertUnallocated(d, child, child->firstSector());

		lastEnd = child->lastSector() + 1;
	}

	// Take care of the free space between the end of the last child and the end
	// of the device or the extended partition.
	qint64 parentEnd = lastUsable();

	if (!p->isRoot())
	{
		Partition* extended = dynamic_cast<Partition*>(p);
		Q_ASSERT(extended != NULL);
		parentEnd = (extended != NULL) ? extended->lastSector() : -1;
	}

	if (parentEnd >= firstUsable())
		p->insert(createUnallocated(d, *p, lastEnd, parentEnd));
}

/** Updates the unallocated Partitions for this PartitionTable.
	@param d the Device this PartitionTable is on
*/
void PartitionTable::updateUnallocated(const Device& d)
{
	removeUnallocated();
	insertUnallocated(d, this, firstUsable());
}

qint64 PartitionTable::defaultFirstUsable(const Device& d, TableType t)
{
	if (t == msdos && Config::useCylinderAlignment())
		return d.sectorsPerTrack();

	return Config::sectorAlignment();
}

qint64 PartitionTable::defaultLastUsable(const Device& d, TableType t)
{
	if (t == gpt)
		return d.totalSectors() - 1 - 32 - 1;

	return d.totalSectors() - 1;
}

static struct
{
	const QString name; /**< name of partition table type */
	quint32 maxPrimaries; /**< max numbers of primary partitions supported */
	bool canHaveExtended; /**< does partition table type support extended partitions */
	bool isReadOnly; /**< does KDE Partition Manager support this only in read only mode */
	PartitionTable::TableType type; /**< enum type */
} tableTypes[] =
{
	{ QStringLiteral("aix"), 4, false, true, PartitionTable::aix },
	{ QStringLiteral("bsd"), 8, false, true, PartitionTable::bsd },
	{ QStringLiteral("dasd"), 1, false, true, PartitionTable::dasd },
	{ QStringLiteral("msdos"), 4, true, false, PartitionTable::msdos },
	{ QStringLiteral("msdos"), 4, true, false, PartitionTable::msdos_sectorbased },
	{ QStringLiteral("dvh"), 16, true, true, PartitionTable::dvh },
	{ QStringLiteral("gpt"), 128, false, false, PartitionTable::gpt },
	{ QStringLiteral("loop"), 1, false, true, PartitionTable::loop },
	{ QStringLiteral("mac"), 0xffff, false, true, PartitionTable::mac },
	{ QStringLiteral("pc98"), 16, false, true, PartitionTable::pc98 },
	{ QStringLiteral("amiga"), 128, false, true, PartitionTable::amiga },
	{ QStringLiteral("sun"), 8, false, true, PartitionTable::sun }
};

PartitionTable::TableType PartitionTable::nameToTableType(const QString& n)
{
	for (size_t i = 0; i < sizeof(tableTypes) / sizeof(tableTypes[0]); i++)
		if (n == tableTypes[i].name)
			return tableTypes[i].type;

	return PartitionTable::unknownTableType;
}

QString PartitionTable::tableTypeToName(TableType l)
{
	for (size_t i = 0; i < sizeof(tableTypes) / sizeof(tableTypes[0]); i++)
		if (l == tableTypes[i].type)
			return tableTypes[i].name;

	return i18nc("@item partition table name", "unknown");
}

qint64 PartitionTable::maxPrimariesForTableType(TableType l)
{
	for (size_t i = 0; i < sizeof(tableTypes) / sizeof(tableTypes[0]); i++)
		if (l == tableTypes[i].type)
			return tableTypes[i].maxPrimaries;

	return 1;
}

bool PartitionTable::tableTypeSupportsExtended(TableType l)
{
	for (size_t i = 0; i < sizeof(tableTypes) / sizeof(tableTypes[0]); i++)
		if (l == tableTypes[i].type)
			return tableTypes[i].canHaveExtended;

	return false;
}

bool PartitionTable::tableTypeIsReadOnly(TableType l)
{
	for (size_t i = 0; i < sizeof(tableTypes) / sizeof(tableTypes[0]); i++)
		if (l == tableTypes[i].type)
			return tableTypes[i].isReadOnly;

	return false;
}

/** Simple heuristic to determine if the PartitionTable is sector aligned (i.e.
	if its Partitions begin at sectors evenly divisable by Config::sectorAlignment().
	@return true if is sector aligned, otherwise false
*/
bool PartitionTable::isSectorBased(const Device& d) const
{
	if (type() == PartitionTable::msdos)
	{
		// return configured default for empty partition tables
		if (numPrimaries() == 0)
			return !Config::useCylinderAlignment();

		quint32 numCylinderAligned = 0;
		quint32 numSectorAligned = 0;

		// see if we have more cylinder aligned partitions than sector
		// aligned ones.
		foreach(const Partition* p, children())
			if (p->firstSector() % Config::sectorAlignment() == 0)
				numSectorAligned++;
			else if (p->firstSector() % d.cylinderSize() == 0)
				numCylinderAligned++;

		return numSectorAligned >= numCylinderAligned;
	}

	return type() == PartitionTable::msdos_sectorbased;
}

void PartitionTable::setType(const Device& d, TableType t)
{
	setFirstUsableSector(defaultFirstUsable(d, t));
	setLastUsableSector(defaultLastUsable(d, t));

	m_Type = t;

	updateUnallocated(d);
}

static bool isPartitionLessThan(const Partition* p1, const Partition* p2)
{
	return p1->number() < p2->number();
}

QTextStream& operator<<(QTextStream& stream, const PartitionTable& ptable)
{
	stream << "type: \"" << ptable.typeName() << "\"\n"
		<< "align: \"" << (ptable.type() == PartitionTable::msdos ? "cylinder" : "sector") << "\"\n"
		<< "\n# number start end type roles label flags\n";

	QList<const Partition*> partitions;

	foreach(const Partition* p, ptable.children())
		if (!p->roles().has(PartitionRole::Unallocated))
		{
			partitions.append(p);

			if (p->roles().has(PartitionRole::Extended))
				foreach(const Partition* child, p->children())
					if (!child->roles().has(PartitionRole::Unallocated))
						partitions.append(child);
		}

	qSort(partitions.begin(), partitions.end(), isPartitionLessThan);

	foreach(const Partition* p, partitions)
		stream << *p;

	return stream;
}
