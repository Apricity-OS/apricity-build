/***************************************************************************
 *   Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                       *
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

#include "gui/formattedspinbox.h"

#include <QLocale>

// private method from Qt sources, qabstractspinbox.h:

QString FormattedSpinBox::stripped(const QString &t, int *pos) const
{
	QString text = t;
	if (specialValueText().size() == 0 || text != specialValueText()) {
		int from = 0;
		int size = text.size();
		bool changed = false;
		if (prefix().size() && text.startsWith(prefix())) {
			from += prefix().size();
			size -= from;
			changed = true;
		}
		if (suffix().size() && text.endsWith(suffix())) {
			size -= suffix().size();
			changed = true;
		}
		if (changed)
			text = text.mid(from, size);
	}

	const int s = text.size();
	text = text.trimmed();
	if (pos)
		(*pos) -= (s - text.size());
	return text;
}

QString FormattedSpinBox::textFromValue(double value) const
{
	return QLocale().toString(value, 'f', decimals());
}

double FormattedSpinBox::valueFromText(const QString& text) const
{
	return QLocale().toDouble(stripped(text));
}
