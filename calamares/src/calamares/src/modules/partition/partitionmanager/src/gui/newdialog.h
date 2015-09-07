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

#if !defined(NEWDIALOG__H)

#define NEWDIALOG__H

#include "gui/sizedialogbase.h"

#include "core/partition.h"

#include "fs/filesystem.h"

class Device;

/** Dialog to create new Partitions.

	Dialog to create a new Partition in some unallocated space on a Device.

	@author Volker Lanz <vl@fidra.de>
*/
class NewDialog : public SizeDialogBase
{
	Q_OBJECT

	public:
		NewDialog(QWidget* parent, Device& device, Partition& unallocatedPartition, PartitionRole::Roles r);
		~NewDialog();

	protected Q_SLOTS:
		void accept();
		void onRoleChanged(bool);
		void onFilesystemChanged(int);
		void onLabelChanged(const QString& newLabel);

	protected:
		void setupConnections();
		void setupDialog();
		void updateHideAndShow();
		void updateFileSystem(FileSystem::Type t);
		PartitionRole::Roles partitionRoles() const { return m_PartitionRoles; }
		virtual bool canGrow() const { return true; }
		virtual bool canShrink() const { return true; }
		virtual bool canMove() const { return true; }

	private:
		PartitionRole::Roles m_PartitionRoles;
};

#endif
