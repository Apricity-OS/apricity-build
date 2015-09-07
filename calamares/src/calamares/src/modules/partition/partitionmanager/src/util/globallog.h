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

#if !defined(GLOBALLOG__H)

#define GLOBALLOG__H

#include "util/libpartitionmanagerexport.h"

#include <QString>
#include <QObject>
#include <qglobal.h>

class LIBPARTITIONMANAGERPRIVATE_EXPORT Log
{
	public:
		enum Level
		{
			debug = 0,
			information = 1,
			warning = 2,
			error = 3
		};

	public:
		Log(Level lev = information) : ref(1), level(lev) {}
		~Log();
		Log(const Log& other) : ref(other.ref + 1), level(other.level) {}

	private:
		quint32 ref;
		Level level;
};

/** Global logging.
	@author Volker Lanz <vl@fidra.de>
*/
class LIBPARTITIONMANAGERPRIVATE_EXPORT GlobalLog : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(GlobalLog)

 	friend class Log;
 	friend Log operator<<(Log l, const QString& s);
 	friend Log operator<<(Log l, qint64 i);

	private:
		GlobalLog() : msg() {}

	Q_SIGNALS:
		void newMessage(Log::Level, const QString&);

	public:
		static GlobalLog* instance();

	private:
		void append(const QString& s) { msg += s; }
		void flush(Log::Level level);

	private:
		QString msg;
};

inline Log operator<<(Log l, const QString& s)
{
	GlobalLog::instance()->append(s);
	return l;
}

inline Log operator<<(Log l, qint64 i)
{
	GlobalLog::instance()->append(QString::number(i));
	return l;
}

#endif
