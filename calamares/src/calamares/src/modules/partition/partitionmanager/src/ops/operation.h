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

#if !defined(OPERATION__H)

#define OPERATION__H

#include <QObject>
#include <QList>
#include <qglobal.h>

class Partition;
class Device;
class OperationStack;
class Job;
class OperationRunner;
class Report;

class QString;
class QIcon;

/** Base class of all Operations.

	An Operation serves two purposes: It is responsible for modifying the device preview to show the
	user a state as if the Operation had already been applied and it is made up of Jobs to actually
	perform what the Operation is supposed to do.

	Most Operations just run a list of Jobs and for that reason do not even overwrite
	Operation::execute(). The more complex Operations, however, need to perform some
	extra tasks in between running Jobs (most notably RestoreOperation and CopyOperation). These do
	overwrite Operation::execute().

	Operations own the objects they deal with in most cases, usually Partitions. But as soon as
	an Operation has been successfully executed, it no longer owns anything, because the
	OperationStack then takes over ownership.

	Some rules for creating new operations that inherit the Operation class:

	<ol>
		<li>
			Don't modify anything in the ctor. The ctor runs before merging operations. If you
			modify anything there, undo and merging will break. Just remember what you're
			supposed to do in the ctor and perform modifications in preview().
		</li>
		<li>
			Do not access the preview partitions and devices in description(). If you do,
			the operation descriptions will be wrong.
		</li>
		<li>
			Don't create or delete objects in preview() or undo() since these will be called
			more than once. Create and delete objects in the ctor and dtor.
		</li>
	</ol>

	@author Volker Lanz <vl@fidra.de>
*/
class Operation : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(Operation)

	friend class OperationStack;
	friend class OperationRunner;

	public:
		/** Status of this Operation */
		enum OperationStatus
		{
			StatusNone = 0,             /**< None yet, can be merged */
			StatusPending,              /**< Pending, can be undone */
			StatusRunning,              /**< Currently running */
			StatusFinishedSuccess,      /**< Successfully finished */
			StatusFinishedWarning,      /**< Finished with warnings */
			StatusError                 /**< Finished with errors */
		};

	protected:
		Operation();
		virtual ~Operation();

	Q_SIGNALS:
		int progress(int);
		void jobStarted(Job*, Operation*);
		void jobFinished(Job*, Operation*);

	public:
		virtual QString iconName() const = 0; /**< @return name of the icon for the Operation */
		virtual QString description() const = 0; /**< @return the Operation's description */
 		virtual void preview() = 0; /**< Apply the Operation to the current preview */
		virtual void undo() = 0; /**< Undo applying the Operation to the current preview */
		virtual bool execute(Report& parent);

		virtual bool targets(const Device&) const = 0;
		virtual bool targets(const Partition&) const = 0;

		virtual OperationStatus status() const { return m_Status; } /**< @return the current status */
		virtual QString statusText() const;
		virtual QIcon statusIcon() const;

		virtual void setStatus(OperationStatus s) { m_Status = s; } /**< @param s the new status */

		qint32 totalProgress() const;

	protected Q_SLOTS:
		void onJobStarted();
		void onJobFinished();

	protected:
		void insertPreviewPartition(Device& targetDevice, Partition& newPartition);
		void removePreviewPartition(Device& device, Partition& p);

		void addJob(Job* job);

		QList<Job*>& jobs() { return m_Jobs; }
		const QList<Job*>& jobs() const { return m_Jobs; }

		void setProgressBase(qint32 i) { m_ProgressBase = i; }
		qint32 progressBase() const { return m_ProgressBase; }

	private:
		OperationStatus m_Status;
		QList<Job*> m_Jobs;
		qint32 m_ProgressBase;
};

#endif
