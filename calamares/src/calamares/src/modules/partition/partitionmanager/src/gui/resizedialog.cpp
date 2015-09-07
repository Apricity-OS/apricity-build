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

#include "gui/resizedialog.h"
#include "gui/sizedialogwidget.h"

#include "core/partition.h"
#include "core/device.h"

#include "fs/filesystem.h"

#include "ops/resizeoperation.h"

#include "util/capacity.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

/** Creates a new ResizeDialog
	@param parent pointer to the parent widget
	@param device the Device the Partition to resize is on
	@param p the Partition to resize
	@param freebefore number of sectors free before the Partition to resize
	@param freeafter number of sectors free after the Partition to resize
*/
ResizeDialog::ResizeDialog(QWidget* parent, Device& d, Partition& p, qint64 minFirst, qint64 maxLast) :
	SizeDialogBase(parent, d, p, minFirst, maxLast),
	m_OriginalFirstSector(p.firstSector()),
	m_OriginalLastSector(p.lastSector()),
	m_ResizedFirstSector(p.firstSector()),
	m_ResizedLastSector(p.lastSector())
{
	setWindowTitle(xi18nc("@title:window", "Resize/move partition: <filename>%1</filename>", partition().deviceNode()));

	dialogWidget().hideRole();
	dialogWidget().hideFileSystem();
	dialogWidget().hideLabel();

	setupDialog();
	setupConstraints();
	setupConnections();

	KConfigGroup kcg(KSharedConfig::openConfig(), "resizeDialog");
	restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));
}

/** Destroys a ResizeDialog */
ResizeDialog::~ResizeDialog()
{
	KConfigGroup kcg(KSharedConfig::openConfig(), "resizeDialog");
	kcg.writeEntry("Geometry", saveGeometry());
}

void ResizeDialog::rollback()
{
	partition().setFirstSector(originalFirstSector());
	partition().fileSystem().setFirstSector(originalFirstSector());

	partition().setLastSector(originalLastSector());
	partition().fileSystem().setLastSector(originalLastSector());

	if (partition().roles().has(PartitionRole::Extended))
	{
		device().partitionTable()->removeUnallocated(&partition());
		device().partitionTable()->insertUnallocated(device(), &partition(), partition().firstSector());
	}
}

void ResizeDialog::accept()
{
	setResizedFirstSector(partition().firstSector());
	setResizedLastSector(partition().lastSector());

	rollback();
	QDialog::accept();
}

void ResizeDialog::reject()
{
	rollback();
	QDialog::reject();
}

void ResizeDialog::setupDialog()
{
	SizeDialogBase::setupDialog();
	okButton->setEnabled(false);
}

void ResizeDialog::setDirty()
{
	okButton->setEnabled(isModified());
}

/** @return true if the user modified anything */
bool ResizeDialog::isModified() const
{
	return partition().firstSector() != originalFirstSector() || partition().lastSector() != originalLastSector();
}

bool ResizeDialog::canGrow() const
{
	return ResizeOperation::canGrow(&partition());
}

bool ResizeDialog::canShrink() const
{
	return ResizeOperation::canShrink(&partition());
}

bool ResizeDialog::canMove() const
{
	return ResizeOperation::canMove(&partition());
}
