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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#if !defined(CREATEPARTITIONTABLEDIALOG__H)

#define CREATEPARTITIONTABLEDIALOG__H

#include "gui/createpartitiontablewidget.h"

#include "core/partitiontable.h"

#include <QDialog>

class Device;
class QDialogButtonBox;
class QPushButton;

class CreatePartitionTableDialog : public QDialog
{
	Q_OBJECT

	public:
		CreatePartitionTableDialog(QWidget* parent, const Device& d);

	public:
		PartitionTable::TableType type() const;

	protected:
		CreatePartitionTableWidget& widget() { return *m_DialogWidget; }
		const CreatePartitionTableWidget& widget() const { return *m_DialogWidget; }
		const Device& device() const { return m_Device; }

	protected Q_SLOTS:
		void onMSDOSToggled(bool on);

	private:
		CreatePartitionTableWidget* m_DialogWidget;
		const Device& m_Device;

		QDialogButtonBox* dialogButtonBox;
		QPushButton* createButton;
		QPushButton* cancelButton;
};


#endif
