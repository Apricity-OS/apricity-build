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

#include "gui/partresizerwidget.h"
#include "gui/partwidget.h"

#include "core/partition.h"
#include "core/device.h"
#include "core/partitiontable.h"
#include "core/partitionalignment.h"

#include "fs/filesystem.h"

#include <QDebug>
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QStyleOptionToolBar>
#include <QStyleOptionFrameV3>
#include <QStyleOptionButton>

const qint32 PartResizerWidget::m_HandleHeight = 59;

/** Creates a new PartResizerWidget

	Initializing is mostly done in init().

	@param parent pointer to the parent widget
*/
PartResizerWidget::PartResizerWidget(QWidget* parent) :
	QWidget(parent),
	m_Device(NULL),
	m_Partition(NULL),
	m_PartWidget(NULL),
	m_MinimumFirstSector(0),
	m_MaximumFirstSector(-1),
	m_MinimumLastSector(-1),
	m_MaximumLastSector(0),
	m_MinimumLength(-1),
	m_MaximumLength(-1),
	m_LeftHandle(this),
	m_RightHandle(this),
	m_DraggedWidget(NULL),
	m_Hotspot(0),
	m_MoveAllowed(true),
	m_ReadOnly(false),
	m_Align(true)
{
}

/** Intializes the PartResizerWidget
	@param d the Device the Partition is on
	@param p the Partition to show and/or resize
	@param minFirst the minimum value for the first sector
	@param maxLast the maximum value for the last sector
*/
void PartResizerWidget::init(Device& d, Partition& p, qint64 minFirst, qint64 maxLast, bool read_only, bool move_allowed)
{
	setDevice(d);
	setPartition(p);

	setMinimumFirstSector(minFirst);
	setMaximumLastSector(maxLast);

	setReadOnly(read_only);
	setMoveAllowed(move_allowed);

	setMinimumLength(qMax(partition().sectorsUsed(), partition().minimumSectors()));
	setMaximumLength(qMin(totalSectors(), partition().maximumSectors()));

	// set margins to accommodate to top/bottom button asymmetric layouts
	QStyleOptionButton bOpt;
	bOpt.initFrom(this);

	QRect buttonRect(style()->subElementRect(QStyle::SE_PushButtonContents, &bOpt));

	int asym = (rect().bottom() - buttonRect.bottom()) - (buttonRect.top() - rect().top());
	if (asym > 0)
		setContentsMargins(0, asym, 0, 0);
	else
		setContentsMargins(0, 0, 0, asym);

	if (!readOnly())
	{
		QPixmap pixmap(handleWidth(), handleHeight());
		pixmap.fill(Qt::transparent);
		QPainter p(&pixmap);
		QStyleOption opt;
		opt.state |= QStyle::State_Horizontal;
		opt.rect = pixmap.rect().adjusted(0, 2, 0, -2);
		style()->drawControl(QStyle::CE_Splitter, &opt, &p, this);

		leftHandle().setPixmap(pixmap);
		rightHandle().setPixmap(pixmap);

		leftHandle().setFixedSize(handleWidth(), handleHeight());
		rightHandle().setFixedSize(handleWidth(), handleHeight());
	}

	delete m_PartWidget;
	m_PartWidget = new PartWidget(this, &partition());

	if (!readOnly())
	{
		leftHandle().setCursor(Qt::SizeHorCursor);
		rightHandle().setCursor(Qt::SizeHorCursor);
	}

	if (moveAllowed())
		partWidget().setCursor(Qt::SizeAllCursor);

	partWidget().setToolTip(QString());

	updatePositions();
}

qint32 PartResizerWidget::handleWidth() const
{
	return style()->pixelMetric(QStyle::PM_SplitterWidth);
}

qint64 PartResizerWidget::sectorsPerPixel() const
{
	return totalSectors() / (width() - 2 * handleWidth());
}

int PartResizerWidget::partWidgetStart() const
{
	return handleWidth() + (partition().firstSector() - minimumFirstSector()) / sectorsPerPixel();
}

int PartResizerWidget::partWidgetWidth() const
{
	return partition().length() / sectorsPerPixel();
}

void PartResizerWidget::updatePositions()
{
	QMargins margins(contentsMargins());

	partWidget().move(partWidgetStart() + margins.left(), margins.top());
	partWidget().resize(partWidgetWidth() - margins.left() - margins.right(), height() - margins.top() - margins.bottom());

	leftHandle().move(partWidgetStart() - leftHandle().width(), 0);

	rightHandle().move(partWidgetStart() + partWidgetWidth(), 0);

	partWidget().update();
}

void PartResizerWidget::resizeEvent(QResizeEvent* event)
{
	updatePositions();
	QWidget::resizeEvent(event);
}

void PartResizerWidget::paintEvent(QPaintEvent*)
{
	// draw sunken frame
	QPainter painter(this);
	QStyleOptionFrameV3 opt;
	opt.initFrom(this);
	opt.frameShape = QFrame::StyledPanel;
	opt.rect = contentsRect();
	opt.lineWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &opt, this);
	opt.midLineWidth = 0;
	opt.state |= QStyle::State_Sunken;

	style()->drawPrimitive(QStyle::PE_PanelLineEdit, &opt, &painter, this);
}

void PartResizerWidget::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_DraggedWidget = static_cast<QWidget*>(childAt(event->pos()));

		if (m_DraggedWidget != NULL)
		{
			if (partWidget().isAncestorOf(m_DraggedWidget))
				m_DraggedWidget = &partWidget();

			m_Hotspot = m_DraggedWidget->mapFromParent(event->pos()).x();
		}
	}
}

bool PartResizerWidget::checkConstraints(qint64 first, qint64 last) const
{
	return (maximumFirstSector() == -1 || first <= maximumFirstSector()) &&
		(minimumFirstSector() == 0 || first >= minimumFirstSector()) &&
		(minimumLastSector() == -1 || last >= minimumLastSector()) &&
		(maximumLastSector() == 0 || last <= maximumLastSector());
}

bool PartResizerWidget::movePartition(qint64 newFirstSector)
{
	const qint64 originalLength = partition().length();
	const bool isLengthAligned = PartitionAlignment::isLengthAligned(device(), partition());

	if (maximumFirstSector(align()) > -1 && newFirstSector > maximumFirstSector(align()))
		newFirstSector = maximumFirstSector(align());

	if (minimumFirstSector(align()) > 0 && newFirstSector < minimumFirstSector(align()))
		newFirstSector = minimumFirstSector(align());

	if (align())
		newFirstSector = PartitionAlignment::alignedFirstSector(device(), partition(), newFirstSector, minimumFirstSector(align()), maximumFirstSector(align()), -1, -1);

	qint64 delta = newFirstSector - partition().firstSector();

	if (delta == 0)
		return false;

	qint64 newLastSector = partition().lastSector() + delta;

	if (minimumLastSector(align()) > -1 && newLastSector < minimumLastSector(align()))
	{
		const qint64 deltaLast = minimumLastSector(align()) - newLastSector;
		newFirstSector += deltaLast;
		newLastSector += deltaLast;
	}

	if (maximumLastSector(align()) > 0 && newLastSector > maximumLastSector(align()))
	{
		const qint64 deltaLast = newLastSector - maximumLastSector(align());
		newFirstSector -= deltaLast;
		newLastSector -= deltaLast;
	}

	if (align())
		newLastSector = PartitionAlignment::alignedLastSector(device(), partition(), newLastSector, minimumLastSector(align()), maximumLastSector(align()), -1, -1, originalLength, isLengthAligned);

	if (newLastSector == partition().lastSector())
		return false;

	if (isLengthAligned && newLastSector - newFirstSector + 1 != partition().length())
	{
		qDebug() << "length changes while trying to move partition " << partition().deviceNode() << ". new first: " << newFirstSector << ", new last: " << newLastSector << ", old length: " << partition().length() << ", new length: " << newLastSector - newFirstSector + 1;
		return false;
	}

	if (!checkConstraints(newFirstSector, newLastSector))
	{
		qDebug() << "constraints not satisfied while trying to move partition " << partition().deviceNode() << ". new first: " << newFirstSector << ", new last: " << newLastSector;
		return false;
	}

	if (align() && !PartitionAlignment::isAligned(device(), partition(), newFirstSector, newLastSector, true))
	{
		qDebug() << "partition " << partition().deviceNode() << " not aligned but supposed to be. new first: " << newFirstSector << " delta: " << PartitionAlignment::firstDelta(device(), partition(), newFirstSector) << ", new last: " << newLastSector << ", delta: " << PartitionAlignment::lastDelta(device(), partition(), newLastSector);
		return false;
	}

	if (partition().children().size() > 0 &&
		(!checkAlignment(*partition().children().first(), partition().firstSector() - newFirstSector) ||
		!checkAlignment(*partition().children().last(), partition().lastSector() - newLastSector)))
	{
		qDebug() << "cannot align children while trying to move partition " << partition().deviceNode();
		return false;
	}

	partition().setFirstSector(newFirstSector);
	partition().fileSystem().setFirstSector(newFirstSector);

	partition().setLastSector(newLastSector);
	partition().fileSystem().setLastSector(newLastSector);

	updatePositions();

	emit firstSectorChanged(partition().firstSector());
	emit lastSectorChanged(partition().lastSector());

	return true;
}

void PartResizerWidget::mouseMoveEvent(QMouseEvent* event)
{
	int x = event->pos().x() - m_Hotspot;

	if (draggedWidget() == &leftHandle())
	{
		const qint64 newFirstSector = qMax(minimumFirstSector() + x * sectorsPerPixel(), 0LL);
		updateFirstSector(newFirstSector);
	}
	else if (draggedWidget() == &rightHandle())
	{
		const qint64 newLastSector = qMin(minimumFirstSector() + (x - rightHandle().width()) * sectorsPerPixel(), maximumLastSector());
		updateLastSector(newLastSector);
	}
	else if (draggedWidget() == &partWidget() && moveAllowed())
	{
		const qint64 newFirstSector = qMax(minimumFirstSector() + (x - handleWidth()) * sectorsPerPixel(), 0LL);
		movePartition(newFirstSector);
	}
}

void PartResizerWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
		m_DraggedWidget = NULL;
}

bool PartResizerWidget::updateFirstSector(qint64 newFirstSector)
{
	if (maximumFirstSector(align()) > -1 && newFirstSector > maximumFirstSector(align()))
		newFirstSector = maximumFirstSector(align());

	if (minimumFirstSector(align()) > 0 && newFirstSector < minimumFirstSector(align()))
		newFirstSector = minimumFirstSector(align());

	const qint64 newLength = partition().lastSector() - newFirstSector + 1;

	if (newLength < minimumLength())
		newFirstSector -= minimumLength() - newLength;

	if (newLength > maximumLength())
		newFirstSector -= newLength - maximumLength();

	if (align())
		newFirstSector = PartitionAlignment::alignedFirstSector(device(), partition(), newFirstSector, minimumFirstSector(align()), maximumFirstSector(align()), minimumLength(), maximumLength());

	if (newFirstSector != partition().firstSector() && (partition().children().size() == 0 || checkAlignment(*partition().children().first(), partition().firstSector() - newFirstSector)))
	{
		const qint64 deltaFirst = partition().firstSector() - newFirstSector;

		partition().setFirstSector(newFirstSector);
		partition().fileSystem().setFirstSector(newFirstSector);

		resizeLogicals(deltaFirst, 0);

		updatePositions();

		emit firstSectorChanged(partition().firstSector());

		return true;
	}

	return false;
}

bool PartResizerWidget::checkAlignment(const Partition& child, qint64 delta) const
{
	// TODO: what is this exactly good for? and is it correct in non-cylinder-aligned
	// situations?
	if (!partition().roles().has(PartitionRole::Extended))
		return true;

	if (child.roles().has(PartitionRole::Unallocated))
		return true;

	return qAbs(delta) >= PartitionAlignment::sectorAlignment(device());
}

void PartResizerWidget::resizeLogicals(qint64 deltaFirst, qint64 deltaLast, bool force)
{
	if (deltaFirst != 0 && partition().children().size() > 0 && partition().children().first()->roles().has(PartitionRole::Unallocated))
	{
		qint64 start = partition().children().first()->firstSector() - deltaFirst;
		qint64 end = partition().children().first()->lastSector() + deltaLast;
		if (PartitionTable::getUnallocatedRange(device(), partition(), start, end))
		{
			partition().children().first()->setFirstSector(start);
			deltaFirst = 0;
		}
	}

	if (deltaLast != 0 && partition().children().size() > 0 && partition().children().last()->roles().has(PartitionRole::Unallocated))
	{
		qint64 start = partition().children().last()->firstSector() - deltaFirst;
		qint64 end = partition().children().last()->lastSector() + deltaLast;
		if (PartitionTable::getUnallocatedRange(device(), partition(), start, end))
		{
			partition().children().last()->setLastSector(end);
			deltaLast = 0;
		}
	}

	if (force || deltaFirst != 0 || deltaLast != 0)
	{
		Q_ASSERT(device().partitionTable());

		device().partitionTable()->removeUnallocated(&partition());

		if (partition().roles().has(PartitionRole::Extended))
			device().partitionTable()->insertUnallocated(device(), &partition(), partition().firstSector());
	}

	partWidget().updateChildren();
}

bool PartResizerWidget::updateLastSector(qint64 newLastSector)
{
	if (minimumLastSector(align()) > -1 && newLastSector < minimumLastSector(align()))
		newLastSector = minimumLastSector(align());

	if (maximumLastSector(align()) > 0 && newLastSector > maximumLastSector(align()))
		newLastSector = maximumLastSector(align());

	const qint64 newLength = newLastSector - partition().firstSector() + 1;

	if (newLength < minimumLength())
		newLastSector += minimumLength() - newLength;

	if (newLength > maximumLength())
		newLastSector -= newLength - maximumLength();

	if (align())
		newLastSector = PartitionAlignment::alignedLastSector(device(), partition(), newLastSector, minimumLastSector(align()), maximumLastSector(align()), minimumLength(), maximumLength());

	if (newLastSector != partition().lastSector() && (partition().children().size() == 0 || checkAlignment(*partition().children().last(), partition().lastSector() - newLastSector)))
	{
		const qint64 deltaLast = newLastSector - partition().lastSector();

		partition().setLastSector(newLastSector);
		partition().fileSystem().setLastSector(newLastSector);

		resizeLogicals(0, deltaLast);
		updatePositions();

		emit lastSectorChanged(partition().lastSector());

		return true;
	}

	return false;
}

/** Sets the minimum sectors the Partition can be long.
	@note This value can never be less than 0 and never be higher than totalSectors()
	@param s the new minimum length
*/
void PartResizerWidget::setMinimumLength(qint64 s)
{
	m_MinimumLength = qBound(0LL, s, totalSectors());
}

/** Sets the maximum sectors the Partition can be long.
	@note This value can never be less than 0 and never by higher than totalSectors()
	@param s the new maximum length
*/
void PartResizerWidget::setMaximumLength(qint64 s)
{
	m_MaximumLength = qBound(0LL, s, totalSectors());
}

/** Sets if moving the Partition is allowed.
	@param b true if moving is allowed
*/
void PartResizerWidget::setMoveAllowed(bool b)
{
	m_MoveAllowed = b;

	if (m_PartWidget != NULL)
		partWidget().setCursor(b ? Qt::SizeAllCursor : Qt::ArrowCursor);
}

qint64 PartResizerWidget::minimumFirstSector(bool aligned) const
{
	if (!aligned || PartitionAlignment::firstDelta(device(), partition(), m_MinimumFirstSector) == 0)
		return m_MinimumFirstSector;

	return m_MinimumFirstSector - PartitionAlignment::firstDelta(device(), partition(), m_MinimumFirstSector) +  PartitionAlignment::sectorAlignment(device());
}

qint64 PartResizerWidget::maximumFirstSector(bool aligned) const
{
	return (m_MaximumFirstSector != -1 && aligned)
		? m_MaximumFirstSector - PartitionAlignment::firstDelta(device(), partition(), m_MaximumFirstSector)
		: m_MaximumFirstSector;
}

qint64 PartResizerWidget::minimumLastSector(bool aligned) const
{
	if (!aligned || PartitionAlignment::lastDelta(device(), partition(), m_MinimumLastSector) == 1)
		return m_MinimumLastSector;

	return m_MinimumLastSector - PartitionAlignment::lastDelta(device(), partition(), m_MinimumLastSector) + 1 + PartitionAlignment::sectorAlignment(device());
}

qint64 PartResizerWidget::maximumLastSector(bool aligned) const
{
	return (m_MaximumLastSector != 0 && aligned)
		? m_MaximumLastSector - PartitionAlignment::lastDelta(device(), partition(), m_MaximumLastSector)
		: m_MaximumLastSector;
}
