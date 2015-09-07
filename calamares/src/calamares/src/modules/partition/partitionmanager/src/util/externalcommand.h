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

#if !defined(EXTERNALCOMMAND__H)

#define EXTERNALCOMMAND__H

#include "util/libpartitionmanagerexport.h"

#include <vector>

#include <QProcess>
#include <QStringList>
#include <QString>
#include <qglobal.h>

class Report;

/** An external command.

	Runs an external command as a child process.

	@author Volker Lanz <vl@fidra.de>
	@author Andrius Štikonas <andrius@stikonas.eu>
*/
class LIBPARTITIONMANAGERPRIVATE_EXPORT ExternalCommand : public QProcess
{
	Q_OBJECT
	Q_DISABLE_COPY(ExternalCommand)

	public:
		explicit ExternalCommand(const QString& cmd = QString(), const QStringList& args = QStringList());
		explicit ExternalCommand(Report& report, const QString& cmd = QString(), const QStringList& args = QStringList());
		explicit ExternalCommand(const std::vector<QString> cmd, const std::vector<QStringList> args);
		explicit ExternalCommand(Report& report, const std::vector<QString> cmd, const std::vector<QStringList> args);
		~ExternalCommand();

	public:
		void setCommand(const std::vector<QString> cmd) { m_Command = cmd; } /**< @param cmd the command to run */
		const std::vector<QString> command() const { return m_Command; } /**< @return the command to run */

		/**	@param s the argument to add
			@param i the command to which the argument is added
		*/
		void addArg(const QString& s, const int i = 0) { m_Args[i] << s; }
		const std::vector<QStringList> args() const { return m_Args; } /**< @return the arguments */
		void setArgs(const std::vector<QStringList> args) { m_Args = args; } /**< @param args the new arguments */

		bool start(int timeout = 30000);
		bool waitFor(int timeout = 30000);
		bool run(int timeout = 30000);

		int exitCode() const { return m_ExitCode; } /**< @return the exit code */

		const QString& output() const { return m_Output; } /**< @return the command output */

		Report* report() { return m_Report; } /**< @return pointer to the Report or NULL */

	protected:
		void setExitCode(int i) { m_ExitCode = i; }
		void setup();

	protected Q_SLOTS:
		void onFinished(int exitCode);
		void onReadOutput();

	private:
		QProcess *processes;
		Report *m_Report;
		std::vector<QString> m_Command;
		std::vector<QStringList> m_Args;
		int m_ExitCode;
		QString m_Output;
};

#endif
