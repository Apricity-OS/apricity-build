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

#if !defined(SMARTDIALOGWIDGET__H)

#define SMARTDIALOGWIDGET__H

#include "ui_smartdialogwidgetbase.h"

class QStyledItemDelegate;
class QPoint;

/** Central widget in the SmartDialogWidget
	@author Volker Lanz <vl@fidra.de>
*/
class SmartDialogWidget : public QWidget, public Ui::SmartDialogWidgetBase
{
	Q_OBJECT

	public:
		SmartDialogWidget(QWidget* parent);
		~SmartDialogWidget();

	public:
		QLabel& statusText() { Q_ASSERT(m_LabelSmartStatusText); return *m_LabelSmartStatusText; }
		QLabel& statusIcon() { Q_ASSERT(m_LabelSmartStatusIcon); return *m_LabelSmartStatusIcon; }
		QLabel& modelName() { Q_ASSERT(m_LabelSmartModelName); return *m_LabelSmartModelName; }
		QLabel& firmware() { Q_ASSERT(m_LabelSmartFirmware); return *m_LabelSmartFirmware; }
		QLabel& serialNumber() { Q_ASSERT(m_LabelSmartSerialNumber); return *m_LabelSmartSerialNumber; }
		QLabel& temperature() { Q_ASSERT(m_LabelSmartTemperature); return *m_LabelSmartTemperature; }
		QLabel& badSectors() { Q_ASSERT(m_LabelSmartBadSectors); return *m_LabelSmartBadSectors; }
		QLabel& poweredOn() { Q_ASSERT(m_LabelSmartPoweredOn); return *m_LabelSmartPoweredOn; }
		QLabel& powerCycles() { Q_ASSERT(m_LabelSmartPowerCycles); return *m_LabelSmartPowerCycles; }

		QLabel& selfTests() { Q_ASSERT(m_LabelSmartSelfTests); return *m_LabelSmartSelfTests; }
		QLabel& overallAssessment() { Q_ASSERT(m_LabelSmartOverallAssessment); return *m_LabelSmartOverallAssessment; }

		QTreeWidget& treeSmartAttributes() { Q_ASSERT(m_TreeSmartAttributes); return *m_TreeSmartAttributes; }
		const QTreeWidget& treeSmartAttributes() const { Q_ASSERT(m_TreeSmartAttributes); return *m_TreeSmartAttributes; }

	protected:
		void setupConnections();
		void loadConfig();
		void saveConfig() const;

	protected Q_SLOTS:
		void onHeaderContextMenu(const QPoint& p);

	private:
		QStyledItemDelegate* m_SmartAttrDelegate;
};

#endif
