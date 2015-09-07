/***************************************************************************
 *   Copyright (C) 2008,2010 by Volker Lanz <vl@fidra.de>                  *
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

#if !defined(JOB__H)

#define JOB__H

#include "fs/filesystem.h"

#include "util/libpartitionmanagerexport.h"

#include <QObject>
#include <qglobal.h>

class QString;
class QIcon;

class CopySource;
class CopyTarget;
class Report;

/** Base class for all Jobs.

	Each Operation is made up of one or more Jobs. Usually, an Operation will run each Job it is
	made up of and only complete successfully if each Job could be run without error. Jobs are
	all-or-nothing and try to be as atomic as possible: A Job is either successfully run or not, there
	is no case where a Job finishes with a warning.

	@author Volker Lanz <vl@fidra.de>
*/
class LIBPARTITIONMANAGERPRIVATE_EXPORT Job : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(Job)

	public:
		/** Status of this Job */
		enum JobStatus
		{
			Pending = 0,		/**< Pending, not yet run */
			Success,			/**< Successfully run */
			Error				/**< Running generated an error */
		};

	protected:
		Job();

	public:
		virtual ~Job() {}

	Q_SIGNALS:
		void started();
		void progress(int);
		void finished();

	public:
		virtual qint32 numSteps() const { return 1; } /**< @return the number of steps the job takes to complete */
		virtual QString description() const = 0; /**< @return the Job's description */
		virtual bool run(Report& parent) = 0; /**< @param parent parent Report to add new child to for this Job @return true if successfully run */

		virtual QIcon statusIcon() const;
		virtual QString statusText() const;

		JobStatus status() const { return m_Status; } /**< @return the Job's current status */

		void emitProgress(int i);

	protected:
		bool copyBlocks(Report& report, CopyTarget& target, CopySource& source);
		bool rollbackCopyBlocks(Report& report, CopyTarget& origTarget, CopySource& origSource);

		Report* jobStarted(Report& parent);
		void jobFinished(Report& report, bool b);

		void setStatus(JobStatus s) { m_Status = s; }

	private:
		JobStatus m_Status;
};

#endif
