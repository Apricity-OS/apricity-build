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

#if !defined(PARTWIDGETBASE__H)

#define PARTWIDGETBASE__H

#include "core/partitionnode.h"

#include <QList>
#include <QWidget>

class Partition;
class PartWidget;
class QWidget;

/** Base class for all widgets that need to position Partitions.
	@author Volker Lanz <vl@fidra.de>
*/
class PartWidgetBase : public QWidget
{
	Q_OBJECT
	Q_DISABLE_COPY(PartWidgetBase)

	protected:
		PartWidgetBase(QWidget* parent) : QWidget(parent) {}
		virtual ~PartWidgetBase() {}

	public:
		virtual qint32 borderWidth() const { return m_BorderWidth; } /**< @return border width */
		virtual qint32 borderHeight() const { return m_BorderHeight; } /**< @return border height */
		static qint32 spacing() { return m_Spacing; } /**< @return spacing between Partitions */
		static qint32 minWidth() { return m_MinWidth; } /**< @return minimum width for a Partition widget */

		virtual QList<PartWidget*> childWidgets();

	protected:
		virtual void positionChildren(const QWidget* destWidget, const PartitionNode::Partitions& partitions, QList<PartWidget*> widgets) const;

	private:
		static const qint32 m_Spacing;
		static const qint32 m_BorderWidth;
		static const qint32 m_BorderHeight;
		static const qint32 m_MinWidth;
};

#endif

