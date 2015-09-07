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

#include "ops/operation.h"

#include "core/partition.h"
#include "core/device.h"

#include "jobs/job.h"

#include "util/report.h"

#include <QDebug>
#include <QIcon>
#include <QString>

#include <KIconThemes/KIconLoader>
#include <KLocalizedString>

Operation::Operation() :
	m_Status(StatusNone),
	m_Jobs(),
	m_ProgressBase(0)
{
}

Operation::~Operation()
{
	qDeleteAll(jobs());
	jobs().clear();
}

void Operation::insertPreviewPartition(Device& device, Partition& p)
{
	Q_ASSERT(device.partitionTable());

	device.partitionTable()->removeUnallocated();

	p.parent()->insert(&p);

	device.partitionTable()->updateUnallocated(device);
}

void Operation::removePreviewPartition(Device& device, Partition& p)
{
	Q_ASSERT(device.partitionTable());

	if (p.parent()->remove(&p))
		device.partitionTable()->updateUnallocated(device);
	else
		qWarning() << "failed to remove partition " << p.deviceNode() << " at " << &p << " from preview.";
}

/** @return text describing the Operation's current status */
QString Operation::statusText() const
{
	static const QString s[] =
	{
		i18nc("@info:progress operation", "None"),
		i18nc("@info:progress operation", "Pending"),
		i18nc("@info:progress operation", "Running"),
		i18nc("@info:progress operation", "Success"),
		i18nc("@info:progress operation", "Warning"),
		i18nc("@info:progress operation", "Error")
	};

	Q_ASSERT(status() >= 0 && static_cast<quint32>(status()) < sizeof(s) / sizeof(s[0]));

	if (status() < 0 || static_cast<quint32>(status()) >= sizeof(s) / sizeof(s[0]))
	{
		qWarning() << "invalid status " << status();
		return QString();
	}

	return s[status()];
}

/** @return icon for the current Operation's status */
QIcon Operation::statusIcon() const
{
	static const QString icons[] =
	{
		QString(),
		QStringLiteral("dialog-information"),
		QStringLiteral("dialog-information"),
		QStringLiteral("dialog-ok"),
		QStringLiteral("dialog-warning"),
		QStringLiteral("dialog-error")
	};

	Q_ASSERT(status() >= 0 && static_cast<quint32>(status()) < sizeof(icons) / sizeof(icons[0]));

	if (status() < 0 || static_cast<quint32>(status()) >= sizeof(icons) / sizeof(icons[0]))
	{
		qWarning() << "invalid status " << status();
		return QIcon();
	}

	if (status() == StatusNone)
		return QIcon();

	return QIcon(KIconLoader().loadIcon(icons[status()], KIconLoader::Small));
}

void Operation::addJob(Job* job)
{
	if (job)
	{
		jobs().append(job);
		connect(job, SIGNAL(started()), SLOT(onJobStarted()));
		connect(job, SIGNAL(progress(int)), SIGNAL(progress(int)));
		connect(job, SIGNAL(finished()), SLOT(onJobFinished()));
	}
}

void Operation::onJobStarted()
{
	Job* job = qobject_cast<Job*>(sender());

	if (job)
		emit jobStarted(job, this);
}

void Operation::onJobFinished()
{
	Job* job = qobject_cast<Job*>(sender());

	if (job)
	{
		setProgressBase(progressBase() + job->numSteps());
		emit jobFinished(job, this);
	}
}

/** @return total number of steps to run this Operation */
qint32 Operation::totalProgress() const
{
	qint32 result = 0;

	foreach (const Job* job, jobs())
		result += job->numSteps();

	return result;
}

/** Execute the operation
	@param parent the parent Report to create a new child for
	@return true on success
*/
bool Operation::execute(Report& parent)
{
	bool rval = false;

	Report* report = parent.newChild(description());

	foreach (Job* job, jobs())
		if (!(rval = job->run(*report)))
			break;

	setStatus(rval ? StatusFinishedSuccess : StatusError);

	report->setStatus(i18nc("@info/plain status (success, error, warning...) of operation", "%1: %2", description(), statusText()));

	return rval;
}
