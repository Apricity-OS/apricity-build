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

#include "gui/newdialog.h"
#include "gui/sizedialogwidget.h"
#include "gui/sizedetailswidget.h"

#include "core/partition.h"
#include "core/device.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"

#include "util/capacity.h"
#include "util/helpers.h"

#include <QFontDatabase>
#include <QtAlgorithms>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

/** Creates a NewDialog
	@param parent the parent widget
	@param device the Device on which a new Partition is to be created
	@param unallocatedPartition the unallocated space on the Device to create a Partition in
	@param r the permitted Roles for the new Partition
*/
NewDialog::NewDialog(QWidget* parent, Device& device, Partition& unallocatedPartition, PartitionRole::Roles r) :
	SizeDialogBase(parent, device, unallocatedPartition, unallocatedPartition.firstSector(), unallocatedPartition.lastSector()),
	m_PartitionRoles(r)
{
	setWindowTitle(i18nc("@title:window", "Create a new partition"));

	setupDialog();
	setupConstraints();
	setupConnections();

	KConfigGroup kcg(KSharedConfig::openConfig(), "newDialog");
	restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));
}

NewDialog::~NewDialog()
{
	KConfigGroup kcg(KSharedConfig::openConfig(), "newDialog");
	kcg.writeEntry("Geometry", saveGeometry());
}

void NewDialog::setupDialog()
{
	QStringList fsNames;
	foreach (const FileSystem* fs, FileSystemFactory::map())
		if (fs->supportCreate() != FileSystem::cmdSupportNone && fs->type() != FileSystem::Extended)
			fsNames.append(fs->name());

	qSort(fsNames.begin(), fsNames.end(), caseInsensitiveLessThan);

	foreach (const QString& fsName, fsNames)
		dialogWidget().comboFileSystem().addItem(createFileSystemColor(FileSystem::typeForName(fsName), 8), fsName);

	QString selected = FileSystem::nameForType(FileSystem::defaultFileSystem());
	const int idx = dialogWidget().comboFileSystem().findText(selected);
	dialogWidget().comboFileSystem().setCurrentIndex(idx != -1 ? idx : 0);

	dialogWidget().radioPrimary().setVisible(partitionRoles() & PartitionRole::Primary);
	dialogWidget().radioExtended().setVisible(partitionRoles() & PartitionRole::Extended);
	dialogWidget().radioLogical().setVisible(partitionRoles() & PartitionRole::Logical);

	if (partitionRoles() & PartitionRole::Primary)
		dialogWidget().radioPrimary().setChecked(true);
	else
		dialogWidget().radioLogical().setChecked(true);

	SizeDialogBase::setupDialog();

	// don't move these above the call to parent's setupDialog, because only after that has
	// run there is a valid partition set in the part resizer widget and they will need that.
	onRoleChanged(false);
	onFilesystemChanged(dialogWidget().comboFileSystem().currentIndex());
}

void NewDialog::setupConnections()
{
	connect(&dialogWidget().radioPrimary(), SIGNAL(toggled(bool)), SLOT(onRoleChanged(bool)));
	connect(&dialogWidget().radioExtended(), SIGNAL(toggled(bool)), SLOT(onRoleChanged(bool)));
	connect(&dialogWidget().radioLogical(), SIGNAL(toggled(bool)), SLOT(onRoleChanged(bool)));
	connect(&dialogWidget().comboFileSystem(), SIGNAL(currentIndexChanged(int)), SLOT(onFilesystemChanged(int)));
	connect(&dialogWidget().label(), SIGNAL(textChanged(const QString&)), SLOT(onLabelChanged(const QString&)));

	SizeDialogBase::setupConnections();
}

void NewDialog::accept()
{
	if (partition().roles().has(PartitionRole::Extended))
	{
		partition().deleteFileSystem();
		partition().setFileSystem(FileSystemFactory::create(FileSystem::Extended, partition().firstSector(), partition().lastSector()));
	}

	QDialog::accept();
}

void NewDialog::onRoleChanged(bool)
{
	PartitionRole::Roles r = PartitionRole::None;

	if (dialogWidget().radioPrimary().isChecked())
		r = PartitionRole::Primary;
	else if (dialogWidget().radioExtended().isChecked())
		r = PartitionRole::Extended;
	else if (dialogWidget().radioLogical().isChecked())
		r = PartitionRole::Logical;

	// Make sure an extended partition gets correctly displayed: Set its file system to extended.
	// Also make sure to set a primary's or logical's file system once the user goes back from
	// extended to any of those.
	if (r == PartitionRole::Extended)
		updateFileSystem(FileSystem::Extended);
	else
		updateFileSystem(FileSystem::typeForName(dialogWidget().comboFileSystem().currentText()));

	dialogWidget().comboFileSystem().setEnabled(r != PartitionRole::Extended);
	partition().setRoles(PartitionRole(r));

	setupConstraints();

	dialogWidget().partResizerWidget().resizeLogicals(0, 0, true);
	dialogWidget().partResizerWidget().update();

	updateHideAndShow();
}

void NewDialog::updateFileSystem(FileSystem::Type t)
{
	partition().deleteFileSystem();
	partition().setFileSystem(FileSystemFactory::create(t, partition().firstSector(), partition().lastSector()));
}

void NewDialog::onFilesystemChanged(int idx)
{
	updateFileSystem(FileSystem::typeForName(dialogWidget().comboFileSystem().itemText(idx)));

	setupConstraints();

	const FileSystem* fs = FileSystemFactory::create(FileSystem::typeForName(dialogWidget().comboFileSystem().currentText()), -1, -1, -1, QString());
	dialogWidget().m_EditLabel->setMaxLength(fs->maxLabelLength());

	updateSpinCapacity(partition().length());
	dialogWidget().partResizerWidget().update();

	updateHideAndShow();
}

void NewDialog::onLabelChanged(const QString& newLabel)
{
	partition().fileSystem().setLabel(newLabel);
}

void NewDialog::updateHideAndShow()
{
	// this is mostly copy'n'pasted from PartPropsDialog::updateHideAndShow()
	if (partition().roles().has(PartitionRole::Extended) || partition().fileSystem().supportSetLabel() == FileSystem::cmdSupportNone)
	{
		dialogWidget().label().setReadOnly(true);
		dialogWidget().noSetLabel().setVisible(true);
		dialogWidget().noSetLabel().setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));

		QPalette palette = dialogWidget().noSetLabel().palette();
		QColor f = palette.color(QPalette::Foreground);
		f.setAlpha(128);
		palette.setColor(QPalette::Foreground, f);
		dialogWidget().noSetLabel().setPalette(palette);
	}
	else
	{
		dialogWidget().label().setReadOnly(false);
		dialogWidget().noSetLabel().setVisible(false);
	}
}
