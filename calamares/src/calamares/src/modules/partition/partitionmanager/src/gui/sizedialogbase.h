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

#if !defined(SIZEDIALOGBASE__H)

#define SIZEDIALOGBASE__H

#include "util/capacity.h"

#include <qglobal.h>
#include <QDebug>
#include <QDialog>
#include <QPushButton>

class Device;
class Partition;
class PartitionTable;
class SizeDialogWidget;
class SizeDetailsWidget;

/** Base class for all dialogs moving or resizing Partitions.
	@author Volker Lanz <vl@fidra.de>
*/
class SizeDialogBase : public QDialog
{
	Q_OBJECT
	Q_DISABLE_COPY(SizeDialogBase)

	protected:
		SizeDialogBase(QWidget* parent, Device& device, Partition& part, qint64 minFirst, qint64 maxLast);
		virtual ~SizeDialogBase() {}

		SizeDialogWidget& dialogWidget() { Q_ASSERT(m_SizeDialogWidget); return *m_SizeDialogWidget; }
		const SizeDialogWidget& dialogWidget() const { Q_ASSERT(m_SizeDialogWidget); return *m_SizeDialogWidget; }

		SizeDetailsWidget& detailsWidget() { Q_ASSERT(m_SizeDetailsWidget); return *m_SizeDetailsWidget; }
		const SizeDetailsWidget& detailsWidget() const { Q_ASSERT(m_SizeDetailsWidget); return *m_SizeDetailsWidget; }

		virtual const PartitionTable& partitionTable() const;
		virtual bool canGrow() const { return true; }
		virtual bool canShrink() const { return true; }
		virtual bool canMove() const { return true; }
		virtual void setupDialog();
		virtual void setupConstraints();
		virtual void setupConnections();
		virtual Partition& partition() { return m_Partition; }
		virtual const Partition& partition() const { return m_Partition; }
		virtual Device& device() { return m_Device; }
		virtual const Device& device() const { return m_Device; }
		virtual qint64 minimumLength() const;
		virtual qint64 maximumLength() const;
		virtual qint64 minimumFirstSector() const { return m_MinimumFirstSector; }
		virtual qint64 maximumLastSector() const { return m_MaximumLastSector; }
		virtual void setDirty() {}
		virtual void updateSpinCapacity(qint64 newLengthInSectors);

		virtual bool align() const;
		virtual qint64 minimumLastSector() const;
		virtual qint64 maximumFirstSector() const;

		virtual void updateSpinFirstSector(qint64 newFirst);
		virtual void updateSpinFreeBefore(qint64 sectorsFreeBefore);

		virtual void updateSpinLastSector(qint64 newLast);
		virtual void updateSpinFreeAfter(qint64 sectorsFreeAfter);

		virtual void setMinimumLength(qint64 s) { m_MinimumLength = s; }
		virtual void setMaximumLength(qint64 s) { m_MaximumLength = s; }

	protected Q_SLOTS:
		void onResizerWidgetFirstSectorChanged(qint64 newFirst);
		void onResizerWidgetLastSectorChanged(qint64 newLast);

		void onSpinCapacityChanged(double newCapacity);
		void onSpinFreeBeforeChanged(double newBefore);
		void onSpinFreeAfterChanged(double newAfter);

		void onSpinFirstSectorChanged(double newFirst);
		void onSpinLastSectorChanged(double newLast);
		void onAlignToggled(bool);

		void toggleDetails();

	protected:
		SizeDialogWidget* m_SizeDialogWidget;
		SizeDetailsWidget* m_SizeDetailsWidget;
		Device& m_Device;
		Partition& m_Partition;
		qint64 m_MinimumFirstSector;
		qint64 m_MaximumLastSector;
		qint64 m_MinimumLength;
		qint64 m_MaximumLength;

	public:
		QPushButton* okButton;
		QPushButton* cancelButton;
		QPushButton* detailsButton;
};

#endif
