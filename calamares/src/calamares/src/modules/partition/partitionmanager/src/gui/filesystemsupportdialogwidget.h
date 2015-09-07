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

#if !defined(FILESYSTEMSUPPORTDIALOGWIDGET__H)

#define FILESYSTEMSUPPORTDIALOGWIDGET__H

#include "ui_filesystemsupportdialogwidgetbase.h"

class FileSystemSupportDialogWidget : public QWidget, public Ui::FileSystemSupportDialogWidgetBase
{
	public:
		FileSystemSupportDialogWidget(QWidget* parent);

	public:
		QTreeWidget& tree() { Q_ASSERT(m_Tree); return *m_Tree; }
		const QTreeWidget& tree() const { Q_ASSERT(m_Tree); return *m_Tree; }
		QPushButton& buttonRescan() { Q_ASSERT(m_ButtonRescan); return *m_ButtonRescan; }
		const QPushButton& buttonRescan() const { Q_ASSERT(m_ButtonRescan); return *m_ButtonRescan; }
};

#endif
