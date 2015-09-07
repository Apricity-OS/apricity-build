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

#include "gui/insertdialog.h"
#include "gui/sizedialogwidget.h"
#include "gui/sizedetailswidget.h"

#include "core/partition.h"

#include "fs/filesystem.h"

#include "ops/resizeoperation.h"

#include <KConfigGroup>
#include <KSharedConfig>
#include <KLocalizedString>

/** Creates a new InsertDialog instance.
	@param parent the parent widget
	@param device the Device the Partition to insert is on
	@param insertedPartition the Partition to insert
	@param destpartition the Partition the new one is to be inserted to
*/
InsertDialog::InsertDialog(QWidget* parent, Device& device, Partition& insertedPartition, const Partition& destpartition) :
	SizeDialogBase(parent, device, insertedPartition, destpartition.firstSector(), destpartition.lastSector()),
	m_DestPartition(destpartition)
{
	setWindowTitle(i18nc("@title:window", "Insert a partition"));

	partition().move(destPartition().firstSector());
	partition().fileSystem().move(destPartition().fileSystem().firstSector());

	dialogWidget().hideRole();
	dialogWidget().hideFileSystem();
	dialogWidget().hideLabel();

	setupDialog();
	setupConstraints();
	setupConnections();

	KConfigGroup kcg(KSharedConfig::openConfig(), "insertDialog");
	restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));
}

/** Destroys an InsertDialog instance */
InsertDialog::~InsertDialog()
{
	KConfigGroup kcg(KSharedConfig::openConfig(), "insertDialog");
	kcg.writeEntry("Geometry", saveGeometry());
}

bool InsertDialog::canGrow() const
{
	return ResizeOperation::canGrow(&partition());
}
