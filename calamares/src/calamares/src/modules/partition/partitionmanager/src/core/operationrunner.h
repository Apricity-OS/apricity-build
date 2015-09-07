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

#if !defined(OPERATIONRUNNER__H)

#define OPERATIONRUNNER__H

#include <QThread>
#include <QMutex>
#include <qglobal.h>

class Operation;
class OperationStack;
class Report;

/** Thread to run the Operations in the OperationStack.

	Runs the OperationStack when the user applies operations.

	@author Volker Lanz <vl@fidra.de>
*/
class OperationRunner : public QThread
{
	Q_OBJECT
	Q_DISABLE_COPY(OperationRunner)

	public:
		OperationRunner(QObject* parent, OperationStack& ostack);

	public:
		void run();
		qint32 numJobs() const;
		qint32 numOperations() const;
		qint32 numProgressSub() const;
		bool isCancelling() const { return m_Cancelling; } /**< @return if the user has requested cancelling */
		void cancel() const { m_Cancelling = true; } /**< Sets cancelling to true. */
		QMutex& suspendMutex() const { return m_SuspendMutex; } /**< @return the QMutex used for syncing */
		QString description(qint32 op) const;
		void setReport(Report* report) { m_Report = report; } /**< @param report the Report to use while running */

	Q_SIGNALS:
		void progressSub(int);
		void opStarted(int, Operation*);
		void opFinished(int, Operation*);
		void finished();
		void cancelled();
		void error();

	protected:
		OperationStack& operationStack() { return m_OperationStack; }
		const OperationStack& operationStack() const { return m_OperationStack; }
		void setCancelling(bool b) { m_Cancelling = b; }
		Report& report() { Q_ASSERT(m_Report); return *m_Report; }

	private:
		OperationStack& m_OperationStack;
		Report* m_Report;
		mutable QMutex m_SuspendMutex;
		mutable volatile bool m_Cancelling;
};

#endif

