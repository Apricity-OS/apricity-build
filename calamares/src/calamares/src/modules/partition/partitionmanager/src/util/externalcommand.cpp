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

#include "util/externalcommand.h"

#include "util/report.h"

#include <cstdlib>

#include <QString>
#include <QStringList>

#include <KLocalizedString>

/** Creates a new ExternalCommand instance without Report.
	@param cmd the command to run
	@param args the arguments to pass to the command
*/
ExternalCommand::ExternalCommand(const QString& cmd, const QStringList& args) :
	m_Report(NULL),
	m_ExitCode(-1),
	m_Output()
{
	m_Command.push_back(cmd);
	m_Args.push_back(args);
	setup();
}

/** Creates a new ExternalCommand instance with Report.
	@param report the Report to write output to.
	@param cmd the command to run
	@param args the arguments to pass to the command
 */
ExternalCommand::ExternalCommand(Report& report, const QString& cmd, const QStringList& args) :
	m_Report(report.newChild()),
	m_ExitCode(-1),
	m_Output()
{
	m_Command.push_back(cmd);
	m_Args.push_back(args);
	setup();
}

/** Creates a new ExternalCommand instance without Report.
	@param cmd the vector of the piped commands to run
	@param args the vector of the arguments to pass to the commands
*/
ExternalCommand::ExternalCommand(const std::vector<QString> cmd, const std::vector<QStringList> args) :
	m_Report(NULL),
	m_Command(cmd),
	m_Args(args),
	m_ExitCode(-1),
	m_Output()
{
	setup();
}

/** Creates a new ExternalCommand instance with Report.
	@param report the Report to write output to.
	@param cmd the vector of the piped commands to run
	@param args the vector of the arguments to pass to the commands
 */
ExternalCommand::ExternalCommand(Report& report, const std::vector<QString> cmd, const std::vector<QStringList> args) :
	m_Report(report.newChild()),
	m_Command(cmd),
	m_Args(args),
	m_ExitCode(-1),
	m_Output()
{
	setup();
}

ExternalCommand::~ExternalCommand()
{
	delete[] processes;
}

void ExternalCommand::setup()
{
	setEnvironment(QStringList() << QStringLiteral("LC_ALL=C") << QStringLiteral("PATH=") + QString::fromUtf8(getenv("PATH")));
	setProcessChannelMode(MergedChannels);

	processes = new QProcess[command().size()];
	connect(&processes[command().size()-1], SIGNAL(readyReadStandardOutput()), SLOT(onReadOutput()));
	connect(&processes[command().size()-1], SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(onFinished(int)));

	for (unsigned int i = 0; i < command().size() - 1; i++)
	{
		processes[i].setStandardOutputProcess(&processes[i+1]);
	}
}

/** Starts the external commands.
	@param timeout timeout to wait for the process to start
	@return true on success
*/
bool ExternalCommand::start(int timeout)
{
	for (unsigned int i = 0; i < command().size(); i++)
		processes[i].start(command()[i], args()[i]);

	if (report())
	{
		QString s;
		for (unsigned int i = 0; i < command().size(); i++)
		{
			s += command()[i] + QStringLiteral(" ") + args()[i].join(QStringLiteral(" "));
			if (i < command().size()-1)
				s += QStringLiteral(" | ");
		}
		report()->setCommand(i18nc("@info/plain", "Command: %1", s));
	}

	for (unsigned int i = 0; i < command().size(); i++)
	{
		if (!processes[i].waitForStarted(timeout))
		{
			if  (report())
				report()->line() << i18nc("@info/plain", "(Command timeout while starting \"%1\")", command()[i] + QStringLiteral(" ") + args()[i].join(QStringLiteral(" ")));

			return false;
		}
	}

	return true;
}

/** Waits for the external commands to finish.
	@param timeout timeout to wait until the process finishes.
	@return true on success
*/
bool ExternalCommand::waitFor(int timeout)
{
	for (unsigned int i = 0; i < command().size(); i++)
	{
		processes[i].closeWriteChannel();

		if (!processes[i].waitForFinished(timeout))
		{
			if  (report())
				report()->line() << i18nc("@info/plain", "(Command timeout while running \"%1\")", command()[i] + QStringLiteral(" ") + args()[i].join(QStringLiteral(" ")));
			return false;
		}
		onReadOutput();
	}
	return true;
}

/** Runs the command.
	@param timeout timeout to use for waiting when starting and when waiting for the process to finish
	@return true on success
*/
bool ExternalCommand::run(int timeout)
{
	return start(timeout) && waitFor(timeout) && exitStatus() == 0;
}

void ExternalCommand::onReadOutput()
{
	const QString s = QString::fromUtf8(processes[command().size()-1].readAllStandardOutput());

	m_Output += s;

	if (report())
		*report() << s;
}

void ExternalCommand::onFinished(int exitCode)
{
	setExitCode(exitCode);
}
