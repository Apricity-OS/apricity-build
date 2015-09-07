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

#include "util/capacity.h"

#include "core/partition.h"
#include "core/device.h"

#include <KFormat>
#include <KLocalizedString>

#include <QDebug>

#include <config.h>

const QString Capacity::m_InvalidString = QStringLiteral("---");

/** Creates a new Capacity instance.
	@param size the size in bytes
*/
Capacity::Capacity(qint64 size) :
	m_Size(size)
{
}

/** Creates a new Capacity instance.
	@param p the Partition
	@param t type of Capacity
*/
Capacity::Capacity(const Partition& p, Type t) :
	m_Size(-1)
{
	switch(t)
	{
		case Used: m_Size = p.used(); break;
		case Available: m_Size = p.available(); break;
		case Total: m_Size = p.capacity(); break;
		default: break;
	}
}

/** Creates a new Capacity instance.
	@param d the Device
*/
Capacity::Capacity(const Device& d) :
	m_Size(d.capacity())
{
}

/** Returns the Capacity as qint64 converted to the given Unit.
	@param u the Unit to use
	@return the Capacity in the given Unit as qint64
*/
qint64 Capacity::toInt(Unit u) const
{
	return static_cast<qint64>(m_Size / unitFactor(Byte, u));
}

/** Returns the Capacity as double converted to the given Unit.
	@param u the Unit to use
	@return the Capacity in the given Unit as double
*/
double Capacity::toDouble(Unit u) const
{
	return static_cast<double>(m_Size) / unitFactor(Byte, u);
}

/** Returns a factor to convert between two Units.
	@param from the Unit to convert from
	@param to the Unit to convert to
	@return the factor to use for conversion
*/
qint64 Capacity::unitFactor(Unit from, Unit to)
{
	Q_ASSERT(from <= to);

	if (from > to)
	{
		qWarning() << "from: " << from << ", to: " << to;
		return 1;
	}

	qint64 result = 1;

	qint32 a = from;
	qint32 b = to;

	while(b-- > a)
		result *= 1024;

	return result;
}

/** Returns the name of a given Unit.
	@param u the Unit to find the name for
	@return the name
*/
QString Capacity::unitName(Unit u, qint64 val)
{
	static QString unitNames[] =
	{
		i18ncp("@info/plain unit", "Byte", "Bytes", val),
		i18nc("@info/plain unit", "KiB"),
		i18nc("@info/plain unit", "MiB"),
		i18nc("@info/plain unit", "GiB"),
		i18nc("@info/plain unit", "TiB"),
		i18nc("@info/plain unit", "PiB"),
		i18nc("@info/plain unit", "EiB"),
		i18nc("@info/plain unit", "ZiB"),
		i18nc("@info/plain unit", "YiB")
	};

	if (static_cast<quint32>(u) >= sizeof(unitNames) / sizeof(unitNames[0]))
		return i18nc("@info/plain unit", "(unknown unit)");

	return unitNames[u];
}

/** Determine if the capacity is valid.
	@return true if it is valid
*/
bool Capacity::isValid() const
{
	return m_Size >= 0;
}

Capacity::Unit Capacity::preferredUnit()
{
	return static_cast<Unit>(Config::preferredUnit());
}

QString Capacity::formatByteSize(double size, int precision)
{
	if (size < 0)
		return invalidString();
	return KFormat().formatByteSize(size, precision);
}
