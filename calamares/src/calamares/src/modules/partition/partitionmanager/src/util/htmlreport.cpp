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

#include "util/htmlreport.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"

#include <QApplication>
#include <QDateTime>
#include <QString>
#include <QTextStream>
#include <QTextDocument>

#include <KAboutData>
#include <KLocalizedString>
#include <kxmlgui_version.h>

#include <sys/utsname.h>
#include <unistd.h>

QString HtmlReport::tableLine(const QString& label, const QString contents)
{
	QString rval;
	QTextStream s(&rval);

	s << "<tr>\n"
		<< QStringLiteral("<td style='font-weight:bold;padding-right:20px;'>%1</td>\n").arg(QString(label).toHtmlEscaped())
		<< QStringLiteral("<td>%1</td>\n").arg(QString(contents).toHtmlEscaped())
		<< "</tr>\n";

	s.flush();

	return rval;
}

QString HtmlReport::header()
{
	QString rval;
	QTextStream s(&rval);

	s <<
		"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"\n"
		"\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
		"<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n"
		"<head>\n"
		"	<title>"
		<< i18n("%1: SMART Status Report", QGuiApplication::applicationDisplayName().toHtmlEscaped())
		<< "</title>\n"
		"	<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/>\n"
		"</head>\n\n"
		"<body>\n";

	s << "<h1>"
		<< i18n("%1: SMART Status Report", QGuiApplication::applicationDisplayName().toHtmlEscaped())
		<< "</h1>\n\n";

	struct utsname info;
	uname(&info);
	const QString unameString = QString::fromUtf8(info.sysname) + QStringLiteral(" ") + QString::fromUtf8(info.nodename) + QStringLiteral(" ") + QString::fromUtf8(info.release) + QStringLiteral(" ") + QString::fromUtf8(info.version) + QStringLiteral(" ") + QString::fromUtf8(info.machine);

	s << "<table>\n"
		<< tableLine(i18n("Date:"), QLocale().toString(QDateTime::currentDateTime(), QLocale::ShortFormat))
		<< tableLine(i18n("Program version:"), QCoreApplication::applicationVersion())
// 		<< tableLine(i18n("Backend:"), QString("%1 (%2)").arg(CoreBackendManager::self()->backend()->about().displayName()).arg(CoreBackendManager::self()->backend()->about().version())) /* FIXME: port KF5 */
		<< tableLine(i18n("KDE Frameworks version:"), QStringLiteral(KXMLGUI_VERSION_STRING))
		<< tableLine(i18n("Machine:"), unameString)
		<< "</table>\n<br/>\n";

	s << "<table>\n";

	s.flush();

	return rval;
}

QString HtmlReport::footer()
{
	return QStringLiteral("\n\n</body>\n</html>\n");
}
