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

#include "core/mountentry.h"

#include <mntent.h>

MountEntry::MountEntry(const QString& n, const QString& p, const QString& t, const QStringList& o, qint32 d, qint32 pn, IdentifyType type) :
	name(n),
	path(p),
	type(t),
	options(o),
	dumpFreq(d),
	passNumber(pn),
	identifyType(type)
{
}

MountEntry::MountEntry(struct mntent* p, IdentifyType type) :
	name(QString::fromUtf8(p->mnt_fsname)),
	path(QString::fromUtf8(p->mnt_dir)),
	type(QString::fromUtf8(p->mnt_type)),
	options(QString::fromUtf8(p->mnt_opts).split(QStringLiteral(","))),
	dumpFreq(p->mnt_freq),
	passNumber(p->mnt_passno),
	identifyType(type)
{
}


