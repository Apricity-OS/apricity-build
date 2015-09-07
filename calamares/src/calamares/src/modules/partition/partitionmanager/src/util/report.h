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

#if !defined(REPORT__H)

#define REPORT__H

#include "util/libpartitionmanagerexport.h"

#include <QObject>
#include <QList>
#include <QString>
#include <qglobal.h>

class ReportLine;

/** Report details about running Operations and Jobs.

	Gather information for the report shown in the ProgressDialog's detail view.

	@author Volker Lanz <vl@fidra.de>
*/
class LIBPARTITIONMANAGERPRIVATE_EXPORT Report : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(Report)

	friend Report& operator<<(Report& report, const QString& s);
	friend Report& operator<<(Report& report, qint64 i);

	public:
		explicit Report(Report* p, const QString& cmd = QString());
		~Report();

	Q_SIGNALS:
		void outputChanged();

	public:
		Report* newChild(const QString& cmd = QString());

		const QList<Report*>& children() const { return m_Children; } /**< @return the list of this Report's children */

		Report* parent() { return m_Parent; } /**< @return pointer to this Reports parent. May be NULL if this is the root Report */
		const Report* parent() const { return m_Parent; } /**< @return pointer to this Reports parent. May be NULL if this is the root Report */

		Report* root();
		const Report* root() const;

		const QString& command() const { return m_Command; } /**< @return the command */
		const QString& output() const { return m_Output; } /**< @return the output */
		const QString& status() const { return m_Status; } /**< @return the status line */

		void setCommand(const QString& s) { m_Command = s; } /**< @param s the new command */
		void setStatus(const QString& s) { m_Status = s; } /**< @param s the new status */
		void addOutput(const QString& s);

		QString toHtml() const;
		QString toText() const;

		ReportLine line();

		static QString htmlHeader();
		static QString htmlFooter();

	protected:
		void emitOutputChanged();

	private:
		Report* m_Parent;
		QList<Report*> m_Children;
		QString m_Command;
		QString m_Output;
		QString m_Status;
};

inline Report& operator<<(Report& report, const QString& s)
{
	report.addOutput(s);
	return report;
}

inline Report& operator<<(Report& report, qint64 i)
{
	report.addOutput(QString::number(i));
	return report;
}

class ReportLine
{
	friend ReportLine operator<<(ReportLine reportLine, const QString& s);
	friend ReportLine operator<<(ReportLine reportLine, qint64 i);
	friend class Report;

	protected:
		ReportLine(Report& r) : ref(1), report(r.newChild()) {}

	public:
		~ReportLine() { if (--ref == 0) *report << QStringLiteral("\n"); }
		ReportLine(const ReportLine& other) : ref(other.ref + 1), report(other.report) {}

	private:
		ReportLine& operator=(const ReportLine&);

	private:
		qint32 ref;
		Report* report;
};

inline ReportLine operator<<(ReportLine reportLine, const QString& s)
{
	*reportLine.report << s;
	return reportLine;
}

inline ReportLine operator<<(ReportLine reportLine, qint64 i)
{
	*reportLine.report << i;
	return reportLine;
}

#endif
