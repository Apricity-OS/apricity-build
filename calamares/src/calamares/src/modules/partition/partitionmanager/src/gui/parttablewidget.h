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

#if !defined(PARTTABLEWIDGET__H)

#define PARTTABLEWIDGET__H

#include "gui/partwidgetbase.h"

#include <QList>
#include <QLabel>

class PartWidget;
class PartitionTable;

class QResizeEvent;
class QMouseEvent;

/** Widget that represents a PartitionTable.
	@author Volker Lanz <vl@fidra.de>
*/
class PartTableWidget : public PartWidgetBase
{
	Q_OBJECT
	Q_DISABLE_COPY(PartTableWidget)

	public:
		PartTableWidget(QWidget* parent);
		virtual qint32 borderWidth() const { return 0; } /**< @return border width */
		virtual qint32 borderHeight() const { return 0; } /**< @return border height */

	public:
		void setPartitionTable(const PartitionTable* ptable);

		PartWidget* activeWidget(); /**< @return the active widget or NULL if none */
		const PartWidget* activeWidget() const; /**< @return the active widget or NULL if none */

		void setActiveWidget(PartWidget* partWidget);
		void setActivePartition(const Partition* p);
		void clear();
		void setReadOnly(bool b) { m_ReadOnly = b; } /**< @param b the new value for read only */
		bool isReadOnly() const { return m_ReadOnly; } /** @return true if the widget is read only */

	Q_SIGNALS:
		void itemSelectionChanged(PartWidget*);
		void itemDoubleClicked(const PartWidget*);

	protected:
		void resizeEvent(QResizeEvent* event);
		void mousePressEvent(QMouseEvent* event);
		void mouseDoubleClickEvent(QMouseEvent* event);

		const PartitionTable* partitionTable() const { return m_PartitionTable; }

		QLabel& labelEmpty() { return m_LabelEmpty; }
		const QLabel& labelEmpty() const { return m_LabelEmpty; }

	private:
		const PartitionTable* m_PartitionTable;
		QLabel m_LabelEmpty;
		bool m_ReadOnly;
};

#endif

