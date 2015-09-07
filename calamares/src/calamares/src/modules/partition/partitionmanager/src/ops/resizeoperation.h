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

#if !defined(RESIZEOPERATION__H)

#define RESIZEOPERATION__H

#include "ops/operation.h"

#include "core/partition.h"

#include <QString>

class Device;
class OperationStack;
class Report;

class CheckFileSystemJob;
class SetPartGeometryJob;
class ResizeFileSystemJob;
class SetPartGeometryJob;
class SetPartGeometryJob;
class MoveFileSystemJob;
class ResizeFileSystemJob;
class CheckFileSystemJob;

/** Resizes a Partition and FileSystem.

	Resize the given Partition and its FileSystem on the given Device so they start with the
	given new start sector and end with the given new last sector.

	@author Volker Lanz <vl@fidra.de>
*/
class ResizeOperation : public Operation
{
	friend class OperationStack;

	Q_OBJECT
	Q_DISABLE_COPY(ResizeOperation)

	protected:
		/** A ResizeOperation can do a combination of things; this enum is used to determine what
		actually is going to be done. It is used so the ResizeOperation can describe itself and
		when it's actually executed. */
		enum ResizeAction
		{
			None = 0,				/**< Nothing */
			MoveLeft = 1,			/**< Move to the left */
			MoveRight = 2,			/**< Move to the right */
			Grow = 4,				/**< Grow */
			Shrink = 8,				/**< Shrink */
			MoveLeftGrow = 5,		/**< Move to the left then grow */
			MoveRightGrow = 6,		/**< Move to the right then grow */
			MoveLeftShrink = 9,		/**< Shrink then move to the left */
			MoveRightShrink = 10	/**< Shrink then move to the right */
		};

	public:
		ResizeOperation(Device& d, Partition& p, qint64 newfirst, qint64 newlast);

	public:
		QString iconName() const { return QStringLiteral("arrow-right-double"); }
		QString description() const;
		bool execute(Report& parent);
		void preview();
		void undo();

		virtual bool targets(const Device& d) const;
		virtual bool targets(const Partition& p) const;

		static bool canGrow(const Partition* p);
		static bool canShrink(const Partition* p);
		static bool canMove(const Partition* p);

	protected:
		Device& targetDevice() { return m_TargetDevice; }
		const Device& targetDevice() const { return m_TargetDevice; }

		Partition& partition() { return m_Partition; }
		const Partition& partition() const { return m_Partition; }

		bool shrink(Report& report);
		bool move(Report& report);
		bool grow(Report& report);

		ResizeAction resizeAction() const;

		qint64 origFirstSector() const { return m_OrigFirstSector; }
		qint64 origLastSector() const { return m_OrigLastSector; }
		qint64 origLength() const { return origLastSector() - origFirstSector() + 1; }

		qint64 newFirstSector() const { return m_NewFirstSector; }
		qint64 newLastSector() const { return m_NewLastSector; }
		qint64 newLength() const { return newLastSector() - newFirstSector() + 1; }

		CheckFileSystemJob* checkOriginalJob() { return m_CheckOriginalJob; }
		SetPartGeometryJob* moveExtendedJob() { return m_MoveExtendedJob; }
		ResizeFileSystemJob* shrinkResizeJob() { return m_ShrinkResizeJob; }
		SetPartGeometryJob* shrinkSetGeomJob() { return m_ShrinkSetGeomJob; }
		SetPartGeometryJob* moveSetGeomJob() { return m_MoveSetGeomJob; }
		MoveFileSystemJob* moveFileSystemJob() { return m_MoveFileSystemJob; }
		ResizeFileSystemJob* growResizeJob() { return m_GrowResizeJob; }
		SetPartGeometryJob* growSetGeomJob() { return m_GrowSetGeomJob; }
		CheckFileSystemJob* checkResizedJob() { return m_CheckResizedJob; }

	private:
		Device& m_TargetDevice;
		Partition& m_Partition;
		const qint64 m_OrigFirstSector;
		const qint64 m_OrigLastSector;
		qint64 m_NewFirstSector;
		qint64 m_NewLastSector;
		CheckFileSystemJob* m_CheckOriginalJob;
		SetPartGeometryJob* m_MoveExtendedJob;
		ResizeFileSystemJob* m_ShrinkResizeJob;
		SetPartGeometryJob* m_ShrinkSetGeomJob;
		SetPartGeometryJob* m_MoveSetGeomJob;
		MoveFileSystemJob* m_MoveFileSystemJob;
		ResizeFileSystemJob* m_GrowResizeJob;
		SetPartGeometryJob* m_GrowSetGeomJob;
		CheckFileSystemJob* m_CheckResizedJob;
};

#endif
