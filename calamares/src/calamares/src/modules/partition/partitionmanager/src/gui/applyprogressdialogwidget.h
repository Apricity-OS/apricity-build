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

#if !defined(APPLYPROGRESSDIALOGWIDGET__H)

#define APPLYPROGRESSDIALOGWIDGET__H

#include "ui_applyprogressdialogwidgetbase.h"

/** Central widget for the ProgressDialog.
	@author Volker Lanz <vl@fidra.de>
*/
class ApplyProgressDialogWidget : public QWidget, public Ui::ApplyProgressDialogWidgetBase
{
	public:
		ApplyProgressDialogWidget(QWidget* parent) : QWidget(parent) { setupUi(this); }

	public:
		QTreeWidget& treeTasks() { Q_ASSERT(m_TreeTasks); return *m_TreeTasks; }
		QProgressBar& progressTotal() { Q_ASSERT(m_ProgressTotal); return *m_ProgressTotal; }
		QProgressBar& progressSub() { Q_ASSERT(m_ProgressSub); return *m_ProgressSub; }
		QLabel& status() { Q_ASSERT(m_LabelStatus); return *m_LabelStatus; }
		QLabel& totalTime() { Q_ASSERT(m_LabelTime); return *m_LabelTime; }
};

#endif
