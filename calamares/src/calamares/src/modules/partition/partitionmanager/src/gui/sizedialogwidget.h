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

#if !defined(SIZEDIALOGWIDGET__H)

#define SIZEDIALOGWIDGET__H

#include "ui_sizedialogwidgetbase.h"

#include <QWidget>

/** Central widget for the SizeDialogBase
	@author Volker Lanz <vl@fidra.de>
*/
class SizeDialogWidget : public QWidget, public Ui::SizeDialogWidgetBase
{
	Q_OBJECT

	public:
		SizeDialogWidget(QWidget* parent) : QWidget(parent), Ui::SizeDialogWidgetBase() { setupUi(this); }

	public:
		PartResizerWidget& partResizerWidget() { Q_ASSERT(m_PartResizerWidget); return *m_PartResizerWidget; }

		QDoubleSpinBox& spinFreeBefore() { Q_ASSERT(m_SpinFreeBefore); return *m_SpinFreeBefore; }
		QDoubleSpinBox& spinFreeAfter() { Q_ASSERT(m_SpinFreeAfter); return *m_SpinFreeAfter; }
		QDoubleSpinBox& spinCapacity() { Q_ASSERT(m_SpinCapacity); return *m_SpinCapacity; }

		QLabel& labelMinSize() { Q_ASSERT(m_LabelMinSize); return *m_LabelMinSize; }
		QLabel& labelMaxSize() { Q_ASSERT(m_LabelMaxSize); return *m_LabelMaxSize; }

		QRadioButton& radioPrimary() { Q_ASSERT(m_RadioPrimary); return *m_RadioPrimary; }
		QRadioButton& radioExtended() { Q_ASSERT(m_RadioExtended); return *m_RadioExtended; }
		QRadioButton& radioLogical() { Q_ASSERT(m_RadioLogical); return *m_RadioLogical; }

		QComboBox& comboFileSystem() { Q_ASSERT(m_ComboFileSystem); return *m_ComboFileSystem; }

		QLabel& textLabel() { Q_ASSERT(m_LabelTextLabel); return *m_LabelTextLabel; }
		QLineEdit& label() { Q_ASSERT(m_EditLabel); return *m_EditLabel; }
		const QLineEdit& label() const { Q_ASSERT(m_EditLabel); return *m_EditLabel; }
		QLabel& noSetLabel() { Q_ASSERT(m_LabelTextNoSetLabel); return *m_LabelTextNoSetLabel; }

		void hideRole() { delete m_LabelRole; m_LabelRole = NULL; delete m_RadioPrimary; m_RadioPrimary = NULL; delete m_RadioExtended; m_RadioExtended = NULL; delete m_RadioLogical; m_RadioLogical = NULL; }
		void hideFileSystem() { delete m_LabelFileSystem; m_LabelFileSystem = NULL; delete m_ComboFileSystem; m_ComboFileSystem = NULL; }
		void hideLabel() { delete m_LabelTextLabel; m_LabelTextLabel = NULL; delete m_EditLabel; m_EditLabel = NULL; delete m_LabelTextNoSetLabel; m_LabelTextNoSetLabel = NULL; }
};

#endif
