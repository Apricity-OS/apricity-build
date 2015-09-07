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

#include "gui/partwidgetbase.h"
#include "gui/partwidget.h"

#include "core/partition.h"

#include <cmath>

const qint32 PartWidgetBase::m_Spacing = 2;
const qint32 PartWidgetBase::m_BorderWidth = 3;
const qint32 PartWidgetBase::m_BorderHeight = 3;
const qint32 PartWidgetBase::m_MinWidth = 30;

template<typename T>
T sum(const QList<T>& list)
{
	T rval = 0;
	foreach(const T& val, list)
		rval += val;
	return rval;
}

bool distributeLostPixels(QList<qint32>& childrenWidth, qint32 lostPixels)
{
	if (lostPixels == 0 || childrenWidth.size() == 0)
		return false;

	while (lostPixels > 0)
		for (qint32 i = 0; i < childrenWidth.size() && lostPixels > 0; i++)
		{
			childrenWidth[i]++;
			lostPixels--;
		}

	return true;
}

bool levelChildrenWidths(QList<qint32>& childrenWidth, const QList<qint32>& minChildrenWidth, const qint32 destWidgetWidth)
{
	if (childrenWidth.size() == 0)
		return false;

	distributeLostPixels(childrenWidth, destWidgetWidth - sum(childrenWidth));

	// if we find out a partition is too narrow, adjust its screen
	// width to its minimum width and increase adjust by how much we had to increase the
	// screen width. thus, in the end, we have the number of pixels we need
	// to find somewhere else in adjust.
	qint32 adjust = 0;
	for (qint32 i = 0; i < childrenWidth.size(); i++)
		if (childrenWidth[i] < minChildrenWidth[i])
		{
			adjust += minChildrenWidth[i] - childrenWidth[i];
			childrenWidth[i] = minChildrenWidth[i];
		}

	// find out how many partitions are wide enough to have their width reduced; we'd love to
	// check for w > minWidth - (pixels_to_reduce_by), but that last value _depends_ on the
	// number we're trying to find here...
	qint32 numReducable = 0;
	for (qint32 i = 0; i < childrenWidth.size(); i++)
		if (childrenWidth[i] > minChildrenWidth[i])
			numReducable++;

	// no need to do anything... or nothing can be done because all are too narrow
	if (adjust == 0 || numReducable == 0)
		return false;

	// if we have adjusted one or more partitions (and not ALL of them, because in that
	// case, nothing will help us), go through the partitions again and reduce the
	// on screen widths of those big enough anyway
	const qint32 reduce = ceil(1.0 * adjust / numReducable);
	for (qint32 i = 0; i < childrenWidth.size(); i++)
		if (childrenWidth[i] > minChildrenWidth[i])
			childrenWidth[i] -= reduce;

	// distribute pixels lost due to rounding errors
	distributeLostPixels(childrenWidth, destWidgetWidth - sum(childrenWidth));

	return true;
}

void PartWidgetBase::positionChildren(const QWidget* destWidget, const PartitionNode::Partitions& partitions, QList<PartWidget*> widgets) const
{
	if (partitions.size() == 0)
		return;

	QList<qint32> childrenWidth;
	QList<qint32> minChildrenWidth;
	const qint32 destWidgetWidth = destWidget->width() - 2 * borderWidth() - (partitions.size() - 1) * spacing();

	if (destWidgetWidth < 0)
		return;

	qint64 totalLength = 0;
	foreach (const Partition* p, partitions)
		totalLength += p->length();

	// calculate unleveled width for each child and store it
	for (int i = 0; i < partitions.size(); i++)
	{
		childrenWidth.append(partitions[i]->length() * destWidgetWidth / totalLength);

		// Calculate the minimum width for the widget. This is easy for primary and logical partitions: they
		// just have a fixed min width (configured in m_MinWidth). But for extended partitions things
		// are not quite as simple. We need to calc the sum of the min widths for each child, taking
		// spacing and borders into account, and add our own min width.
		qint32 min = (minWidth() + 2 * borderWidth() + spacing()) * partitions[i]->children().size() - spacing() + 2 * borderWidth();

		// if it's too small, this partition is a primary or logical so just use the configured value
		if (min < minWidth())
			min = minWidth();
		minChildrenWidth.append(min);
	}

	// now go level the widths as long as required
 	while (levelChildrenWidths(childrenWidth, minChildrenWidth, destWidgetWidth))
		;

	// move the children to their positions and resize them
	for (int i = 0, x = borderWidth(); i < widgets.size(); i++)
	{
		widgets[i]->setMinimumWidth(minChildrenWidth[i]);
		widgets[i]->move(x, borderHeight());
		widgets[i]->resize(childrenWidth[i], destWidget->height() - 2 * borderHeight());
		x += childrenWidth[i] + spacing();
	}
}

QList<PartWidget*> PartWidgetBase::childWidgets()
{
	QList<PartWidget*> rval;

	foreach(QObject* o, children())
		if (PartWidget* w = qobject_cast<PartWidget*>(o))
			rval.append(w);

	return rval;
}
