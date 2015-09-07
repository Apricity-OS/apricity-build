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

#include "core/operationrunner.h"

#include "core/operationstack.h"

#include "ops/operation.h"

#include "util/report.h"

#include <QMutex>

/** Constructs an OperationRunner.
	@param ostack the OperationStack to act on
*/
OperationRunner::OperationRunner(QObject* parent, OperationStack& ostack) :
	QThread(parent),
	m_OperationStack(ostack),
	m_Report(NULL),
	m_SuspendMutex(),
	m_Cancelling(false)
{
}

/** Runs the operations in the OperationStack. */
void OperationRunner::run()
{
	Q_ASSERT(m_Report);

	setCancelling(false);

	bool status = true;

	for (int i = 0; i < numOperations(); i++)
	{
		suspendMutex().lock();

		if (!status || isCancelling())
		{
			suspendMutex().unlock();
			break;
		}

		Operation* op = operationStack().operations()[i];
		op->setStatus(Operation::StatusRunning);

		emit opStarted(i + 1, op);

		connect(op, SIGNAL(progress(int)), this, SIGNAL(progressSub(int)));

		status = op->execute(report());
		op->preview();

		disconnect(op, SIGNAL(progress(int)), this, SIGNAL(progressSub(int)));

		emit opFinished(i + 1, op);

		suspendMutex().unlock();

		// Sleep a little to give others a chance to suspend us. This should normally not be
		// required -- is it possible that the compiler just optimizes our unlock/lock away
		// if we don't sleep?
		msleep(5);
	}

	if (!status)
		emit error();
	else if (isCancelling())
		emit cancelled();
	else
		emit finished();
}

/** @return the number of Operations to run */
qint32 OperationRunner::numOperations() const
{
	return operationStack().operations().size();
}

/** @return the number of Jobs to run */
qint32 OperationRunner::numJobs() const
{
	qint32 result = 0;

	foreach (const Operation* op,  operationStack().operations())
		result += op->jobs().size();

	return result;
}

/** @param op the number of the Operation to get a description for
	@return the Operation's description
*/
QString OperationRunner::description(qint32 op) const
{
	Q_ASSERT(op >= 0);
	Q_ASSERT(op < operationStack().size());

	return operationStack().operations()[op]->description();
}
