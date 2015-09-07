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

#if !defined(PARTPROPSDIALOG__H)

#define PARTPROPSDIALOG__H

#include "fs/filesystem.h"

#include "core/partition.h"
#include "core/partitiontable.h"

#include <QDialog>

class Device;
class PartPropsWidget;

class QDialogButtonBox;
class QVBoxLayout;
class QWidget;
class QString;


/** Show Partition properties.

	Dialog that shows a Partition's properties and allows the user to change (or recreate)
	the Partition's FileSystem, its label and its flags.

	@author Volker Lanz <vl@fidra.de>
*/
class PartPropsDialog : public QDialog
{
	Q_OBJECT
	Q_DISABLE_COPY(PartPropsDialog)

	public:
		PartPropsDialog(QWidget* parent, Device& d, Partition& p);
		~PartPropsDialog();

	public:
		QString newLabel() const;
		PartitionTable::Flags newFlags() const;
		FileSystem::Type newFileSystemType() const;
		bool forceRecreate() const { return m_ForceRecreate; } /**< @return true if user wants to recreate the FileSystem on the Partition */

	protected:
		void setupDialog();
		void setupConnections();
		void setupFileSystemComboBox();
		void setupFlagsList();
		void updateHideAndShow();

		bool warnFileSystemChange() const { return m_WarnFileSystemChange; }
		void setWarnFileSystemChange() { m_WarnFileSystemChange = true; }

		Device& device() { return m_Device; }
		const Device& device() const { return m_Device; }

		Partition& partition() { return m_Partition; }
		const Partition& partition() const { return m_Partition; }

		PartPropsWidget& dialogWidget() { Q_ASSERT(m_DialogWidget); return *m_DialogWidget; }
		const PartPropsWidget& dialogWidget() const { Q_ASSERT(m_DialogWidget); return *m_DialogWidget; }

		bool isReadOnly() const { return m_ReadOnly; }
		void setForceRecreate(bool b) { m_ForceRecreate = b; }

		void updatePartitionFileSystem();

	protected Q_SLOTS:
		void setDirty();
		void onFilesystemChanged(int idx);
		void onRecreate(int);

	private:
		// m_Device and m_Partition cannot be const because the PartResizerWidget takes
		// both as nonconst even in read only mode (which is a bad design flaw and should be
		// fixed by splitting it in a PartDisplayWidget that is read-only and a PartResizerWidget
		// subclassed from that, maybe)
		Device& m_Device;
		// m_Partition is also not a reference because we want to be able to change it and
		// forget the changes if the user cancels the dialog
		Partition m_Partition;
		bool m_WarnFileSystemChange;
		PartPropsWidget* m_DialogWidget;
		bool m_ReadOnly;
		bool m_ForceRecreate;

		QDialogButtonBox* dialogButtonBox;
		QPushButton* okButton;
		QPushButton* cancelButton;
		QVBoxLayout *mainLayout;

};

#endif
