/***************************************************************************
 *   Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                       *
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

#include "gui/smartdialogwidget.h"

#include "util/helpers.h"

#include <KLocalizedString>

#include <QStyledItemDelegate>
#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QPoint>

#include <config.h>

class SmartAttrDelegate : public QStyledItemDelegate
{
	public:
		SmartAttrDelegate() : QStyledItemDelegate() {}

		virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

void SmartAttrDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QString text = index.data().toString();

	painter->save();

	QStyleOptionViewItemV4 opt = option;
	initStyleOption(&opt, index);

	QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter);

	QTextDocument doc;
	doc.setHtml(text);
	doc.setPageSize(option.rect.size());

	painter->setClipRect(option.rect);
	qint32 offset = (option.rect.height() - doc.size().height()) / 2;
	if (offset < 0)
		offset = 0;

	painter->translate(option.rect.x(), option.rect.y() + offset);
	doc.drawContents(painter);

	painter->restore();
}

SmartDialogWidget::SmartDialogWidget(QWidget* parent) :
	QWidget(parent),
	m_SmartAttrDelegate(new SmartAttrDelegate())
{
	setupUi(this);
	setupConnections();

	loadConfig();

	treeSmartAttributes().setItemDelegateForColumn(1, m_SmartAttrDelegate);
	treeSmartAttributes().header()->setContextMenuPolicy(Qt::CustomContextMenu);
}

SmartDialogWidget::~SmartDialogWidget()
{
	saveConfig();
	delete m_SmartAttrDelegate;
}

void SmartDialogWidget::loadConfig()
{
	QList<int> colWidths = Config::treeSmartAttributesColumnWidths();
	QList<int> colPositions = Config::treeSmartAttributesColumnPositions();
	QList<int> colVisible = Config::treeSmartAttributesColumnVisible();
	QHeaderView* header = treeSmartAttributes().header();

	for (int i = 0; i < treeSmartAttributes().columnCount(); i++)
	{
		if (colPositions[0] != -1 && colPositions.size() > i)
			header->moveSection(header->visualIndex(i), colPositions[i]);

		if (colVisible[0] != -1 && colVisible.size() > i)
			treeSmartAttributes().setColumnHidden(i, colVisible[i] == 0);

		if (colWidths[0] != -1 && colWidths.size() > i)
			treeSmartAttributes().setColumnWidth(i, colWidths[i]);
	}
}

void SmartDialogWidget::saveConfig() const
{
	QList<int> colWidths;
	QList<int> colPositions;
	QList<int> colVisible;

	for (int i = 0; i < treeSmartAttributes().columnCount(); i++)
	{
		colPositions.append(treeSmartAttributes().header()->visualIndex(i));
		colVisible.append(treeSmartAttributes().isColumnHidden(i) ? 0 : 1);
		colWidths.append(treeSmartAttributes().columnWidth(i));
	}

	Config::setTreeSmartAttributesColumnPositions(colPositions);
	Config::setTreeSmartAttributesColumnVisible(colVisible);
	Config::setTreeSmartAttributesColumnWidths(colWidths);

	Config::self()->writeConfig();
}

void SmartDialogWidget::setupConnections()
{
	connect(treeSmartAttributes().header(), SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(onHeaderContextMenu(const QPoint&)));
}

void SmartDialogWidget::onHeaderContextMenu(const QPoint& p)
{
	showColumnsContextMenu(p, treeSmartAttributes());
}
