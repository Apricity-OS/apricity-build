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

#if !defined(CAPACITY__H)

#define CAPACITY__H

class Partition;
class Device;

#include <qglobal.h>

/** Represent any kind of capacity.

	Any kind of capacity that can be expressed in units of Byte, KiB, MiB and so on. Also prints
	capacities in nicely formatted ways.

	@author Volker Lanz <vl@fidra.de>
*/
class Capacity
{
	public:
		/** Units we can deal with */
		enum Unit { Byte, KiB, MiB, GiB, TiB, PiB, EiB, ZiB, YiB };
		/** Type of capacity to print */
		enum Type { Used, Available, Total };
		/** Flags for printing */
		enum Flag { NoFlags = 0, AppendUnit = 1, AppendBytes = 2 };
		Q_DECLARE_FLAGS(Flags, Flag)

	public:
		explicit Capacity(qint64 size);
		explicit Capacity(const Partition& p, Type t = Total);
		Capacity(const Device& d);

	public:
		bool operator==(const Capacity& other) const { return other.m_Size == m_Size; }
		bool operator!=(const Capacity& other) const { return other.m_Size != m_Size; }
		bool operator>(const Capacity& other) const { return other.m_Size > m_Size; }
		bool operator<(const Capacity& other) const { return other.m_Size < m_Size; }
		bool operator>=(const Capacity& other) const { return other.m_Size >= m_Size; }
		bool operator<=(const Capacity& other) const { return other.m_Size <= m_Size; }

		qint64 toInt(Unit u) const;
		double toDouble(Unit u) const;

		bool isValid() const;

		static QString formatByteSize(double size, int precision = 2);
		static const QString& invalidString() { return m_InvalidString; } /**< @return string representing an invalid capacity */
		static QString unitName(Unit u, qint64 val = 1);
		static qint64 unitFactor(Unit from, Unit to);
		static Unit preferredUnit();

	private:
		qint64 m_Size;
		static const QString m_InvalidString;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Capacity::Flags)

#endif
