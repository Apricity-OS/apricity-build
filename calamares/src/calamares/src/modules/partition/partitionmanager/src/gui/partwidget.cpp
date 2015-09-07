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

#include "gui/partwidget.h"

#include "util/capacity.h"

#include "core/partition.h"

#include "fs/filesystem.h"

#include <QPainter>
#include <QStyleOptionButton>
#include <QApplication>
#include <QFontDatabase>

#include <config.h>

/** Creates a new PartWidget
	@param parent pointer to the parent widget
	@param p pointer to the Partition this widget will show. must not be NULL.
*/
PartWidget::PartWidget(QWidget* parent, const Partition* p) :
	PartWidgetBase(parent),
	m_Partition(NULL),
	m_Active(false)
{
	setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));

	// Check if user is running a GTK style; in that case, use plastique as a fallback
	// style for the PartWidget to work around GTK styles not showing the FS colors
	// correctly.
	// Inspired by Aurélien Gâteau's similar workaround in Gwenview (230aebbd)
	//FIXME: port KF5. Is this still necessary?
	/*if (qstrcmp(QApplication::style()->metaObject()->className(), "QGtkStyle") == 0)
	{
		QStyle* style = new QPlastiqueStyle();
		style->setParent(this);
		setStyle(style);
	}*/

	init(p);
}

void PartWidget::init(const Partition* p)
{
	m_Partition = p;

	if (partition())
		setToolTip(partition()->deviceNode() + QStringLiteral("\n") + partition()->fileSystem().name() + QStringLiteral(" ") + QString(Capacity::formatByteSize(partition()->capacity())));
	else
		setToolTip(QString());

	updateChildren();
}

/** Updates the widget's children */
void PartWidget::updateChildren()
{
	if (partition())
	{
		foreach (QWidget* w, childWidgets())
		{
			w->setVisible(false);
			w->deleteLater();
			w->setParent(NULL);
		}

		foreach(const Partition* child, partition()->children())
		{
			QWidget* w = new PartWidget(this, child);
			w->setVisible(true);
		}

		positionChildren(this, partition()->children(), childWidgets());
	}
}

void PartWidget::resizeEvent(QResizeEvent*)
{
	if (partition())
		positionChildren(this, partition()->children(), childWidgets());
}

QColor PartWidget::activeColor(const QColor& col) const
{
#ifdef CALAMARES
	// Hack: Calamares provides the colors for the partition through the
	// QPalette::Button color role, so we overwrite the `col` argument.
	QColor c = palette().button().color();
	return isActive() ? c.darker(190) : c;
#else
	return isActive() ? col.darker(190) : col;
#endif
}

void PartWidget::paintEvent(QPaintEvent*)
{
	if (partition() == NULL)
		return;

	const int usedPercentage = partition()->used() * 100 / partition()->capacity();
	const int w = width() * usedPercentage / 100;

	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing);

	if (partition()->roles().has(PartitionRole::Extended))
	{
		drawGradient(&painter, activeColor(Config::fileSystemColorCode(partition()->fileSystem().type())), QRect(0, 0, width(), height()));
		return;
	}

	const QColor base = activeColor(Config::fileSystemColorCode(partition()->fileSystem().type()));

	if (!partition()->roles().has(PartitionRole::Unallocated))
	{
		const QColor dark = base.darker(105);
		const QColor light = base.lighter(120);

		// draw free space background
		drawGradient(&painter, light, QRect(0, 0, width(), height()), isActive());

		// draw used space in front of that
		drawGradient(&painter, dark, QRect(0, 0, w, height() - 1));
	}
	else
		drawGradient(&painter, base, QRect(0, 0, width(), height()), isActive());

	// draw name and size
	QString text = partition()->deviceNode().remove(QStringLiteral("/dev/")) + QStringLiteral("\n") + QString(Capacity::formatByteSize(partition()->capacity()));

	const QRect textRect(0, 0, width() - 1, height() - 1);
	const QRect boundingRect = painter.boundingRect(textRect, Qt::AlignVCenter | Qt::AlignHCenter, text);
	if (boundingRect.x() > PartWidgetBase::borderWidth() && boundingRect.y() > PartWidgetBase::borderHeight())
	{
		if (isActive())
			painter.setPen(QColor(255, 255, 255));
		painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignHCenter, text);
	}
}

void PartWidget::drawGradient(QPainter* painter, const QColor& color, const QRect& rect, bool active) const
{
	if (rect.width() < 8)
		return;

	QStyleOptionButton option;
	option.initFrom(this);
	option.rect = rect;
	option.palette.setColor(QPalette::Button, color);
	option.palette.setColor(QPalette::Window, color);
	option.state |= QStyle::State_Raised;
	if (!active)
		option.state &= ~QStyle::State_MouseOver;
	else
		option.state |= QStyle::State_MouseOver;

	style()->drawControl(QStyle::CE_PushButtonBevel, &option, painter, this);
}
