/***************************************************************************
 *   Copyright (C) 2009,2010 by Volker Lanz <vl@fidra.de>                  *
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

#if !defined(MOUNTENTRY__H)

#define MOUNTENTRY__H

#include <QString>
#include <QStringList>
#include <qglobal.h>

struct mntent;

class MountEntry
{
	public:
		enum IdentifyType { deviceNode, uuid, label };

	public:
		MountEntry(const QString& n, const QString& p, const QString& t, const QStringList& o, qint32 d, qint32 pn, IdentifyType type);
		MountEntry(struct mntent* p, IdentifyType type);

	public:
		QString name;
		QString path;
		QString type;
		QStringList options;
		qint32 dumpFreq;
		qint32 passNumber;
		IdentifyType identifyType;
};

#endif
