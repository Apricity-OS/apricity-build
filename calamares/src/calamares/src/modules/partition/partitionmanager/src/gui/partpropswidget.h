/***************************************************************************
 *   Copyright (C) 2008,2009 by Volker Lanz <vl@fidra.de>                  *
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

#if !defined(PARTPROPSWIDGET__H)

#define PARTPROPSWIDGET__H

#include "ui_partpropswidgetbase.h"

/** Central widget in the PartPropsDialog.
	@author Volker Lanz <vl@fidra.de>
*/
class PartPropsWidget : public QWidget, public Ui::PartPropsWidgetBase
{
	public:
		PartPropsWidget(QWidget* parent) : QWidget(parent) { setupUi(this); }

	public:
		PartWidget& partWidget() { Q_ASSERT(m_PartWidget); return *m_PartWidget; }

		QLabel& mountPoint() { Q_ASSERT(m_LabelMountPoint); return *m_LabelMountPoint; }
		QLabel& role() { Q_ASSERT(m_LabelRole); return *m_LabelRole; }
		QLabel& capacity() { Q_ASSERT(m_LabelCapacity); return *m_LabelCapacity; }

		QLabel& textAvailable() { Q_ASSERT(m_LabelTextAvailable); return *m_LabelTextAvailable; }
		QLabel& available() { Q_ASSERT(m_LabelAvailable); return *m_LabelAvailable; }

		QLabel& textUsed() { Q_ASSERT(m_LabelTextUsed); return *m_LabelTextUsed; }
		QLabel& used() { Q_ASSERT(m_LabelUsed); return *m_LabelUsed; }

		QLabel& textFileSystem() { Q_ASSERT(m_LabelFileSystem); return *m_LabelFileSystem; }
		QComboBox& fileSystem() { Q_ASSERT(m_ComboFileSystem); return *m_ComboFileSystem; }
		const QComboBox& fileSystem() const { Q_ASSERT(m_ComboFileSystem); return *m_ComboFileSystem; }

		QCheckBox& checkRecreate() { Q_ASSERT(m_CheckRecreate); return *m_CheckRecreate; }

		QLabel& firstSector() { Q_ASSERT(m_LabelFirstSector); return *m_LabelFirstSector; }
		QLabel& lastSector() { Q_ASSERT(m_LabelLastSector); return *m_LabelLastSector; }
		QLabel& numSectors() { Q_ASSERT(m_LabelNumSectors); return *m_LabelNumSectors; }
		QLabel& status() { Q_ASSERT(m_LabelStatus); return *m_LabelStatus; }

		QLabel& textUuid() { Q_ASSERT(m_LabelTextUuid); return *m_LabelTextUuid; }
		QLabel& uuid() { Q_ASSERT(m_LabelUuid); return *m_LabelUuid; }

		QLabel& textLabel() { Q_ASSERT(m_LabelTextLabel); return *m_LabelTextLabel; }
		QLineEdit& label() { Q_ASSERT(m_EditLabel); return *m_EditLabel; }
		const QLineEdit& label() const { Q_ASSERT(m_EditLabel); return *m_EditLabel; }
		QLabel& noSetLabel() { Q_ASSERT(m_LabelTextNoSetLabel); return *m_LabelTextNoSetLabel; }

		QLabel& textFlags() { Q_ASSERT(m_LabelTextFlags); return *m_LabelTextFlags; }
		QListWidget& listFlags() { Q_ASSERT(m_ListFlags); return *m_ListFlags; }
		const QListWidget& listFlags() const { Q_ASSERT(m_ListFlags); return *m_ListFlags; }
		QFrame& lineFlags() { Q_ASSERT(m_LineListFlags); return *m_LineListFlags; }

		void showAvailable(bool b) { available().setVisible(b); textAvailable().setVisible(b); }
		void showUsed(bool b) { used().setVisible(b); textUsed().setVisible(b); }
		void showFileSystem(bool b) { fileSystem().setVisible(b); textFileSystem().setVisible(b); }
		void showCheckRecreate(bool b) { checkRecreate().setVisible(b); }
		void showListFlags(bool b) { listFlags().setVisible(b); textFlags().setVisible(b); lineFlags().setVisible(b); }
		void showLabel(bool b) { textLabel().setVisible(b); label().setVisible(b); }
		void showUuid(bool b) { textUuid().setVisible(b); uuid().setVisible(b); }
};

#endif
