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

#include "gui/partpropsdialog.h"
#include "gui/partpropswidget.h"

#include "core/partition.h"
#include "core/device.h"

#include "fs/filesystemfactory.h"

#include "util/capacity.h"
#include "util/helpers.h"

#include <QComboBox>
#include <QFontDatabase>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QtAlgorithms>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

/** Creates a new PartPropsDialog
	@param parent pointer to the parent widget
	@param d the Device the Partition is on
	@param p the Partition to show properties for
*/
PartPropsDialog::PartPropsDialog(QWidget* parent, Device& d, Partition& p) :
	QDialog(parent),
	m_Device(d),
	m_Partition(p),
	m_WarnFileSystemChange(false),
	m_DialogWidget(new PartPropsWidget(this)),
	m_ReadOnly(partition().isMounted() || partition().state() == Partition::StateCopy || partition().state() == Partition::StateRestore || d.partitionTable()->isReadOnly()),
	m_ForceRecreate(false)
{
	mainLayout = new QVBoxLayout(this);
	setLayout(mainLayout);
	mainLayout->addWidget(&dialogWidget());

	setWindowTitle(xi18nc("@title:window", "Partition properties: <filename>%1</filename>", partition().deviceNode()));

	setupDialog();
	setupConnections();

	KConfigGroup kcg(KSharedConfig::openConfig(), "partPropsDialog");
	restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));
}

/** Destroys a PartPropsDialog */
PartPropsDialog::~PartPropsDialog()
{
	KConfigGroup kcg(KSharedConfig::openConfig(), "partPropsDialog");
	kcg.writeEntry("Geometry", saveGeometry());
}

/** @return the new label */
QString PartPropsDialog::newLabel() const
{
	return dialogWidget().label().text();
}

/** @return the new Partition flags */
PartitionTable::Flags PartPropsDialog::newFlags() const
{
	PartitionTable::Flags flags;

	for (int i = 0; i < dialogWidget().listFlags().count(); i++)
		if (dialogWidget().listFlags().item(i)->checkState() == Qt::Checked)
			flags |= static_cast<PartitionTable::Flag>(dialogWidget().listFlags().item(i)->data(Qt::UserRole).toInt());

	return flags;
}

/** @return the new FileSystem type */
FileSystem::Type PartPropsDialog::newFileSystemType() const
{
	return FileSystem::typeForName(dialogWidget().fileSystem().currentText());
}

void PartPropsDialog::setupDialog()
{
	dialogButtonBox = new QDialogButtonBox;
	okButton = dialogButtonBox->addButton( QDialogButtonBox::Ok );
	cancelButton = dialogButtonBox->addButton( QDialogButtonBox::Cancel );
	mainLayout->addWidget(dialogButtonBox);
	okButton->setEnabled(false);
	cancelButton->setFocus();
	cancelButton->setDefault(true);
	connect(dialogButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(dialogButtonBox, SIGNAL(rejected()), this, SLOT(reject()));

	dialogWidget().partWidget().init(&partition());

	const QString mp = partition().mountPoint().isEmpty()
			? i18nc("@item mountpoint", "(none found)")
			: partition().mountPoint();
	dialogWidget().mountPoint().setText(mp);

	dialogWidget().role().setText(partition().roles().toString());

	QString statusText = i18nc("@label partition state", "idle");
	if (partition().isMounted())
	{
		if (partition().roles().has(PartitionRole::Extended))
			statusText = i18nc("@label partition state", "At least one logical partition is mounted.");
		else if (!partition().mountPoint().isEmpty())
			statusText = xi18nc("@label partition state", "mounted on <filename>%1</filename>", mp);
		else
			statusText = i18nc("@label partition state", "mounted");
	}

	dialogWidget().status().setText(statusText);
	dialogWidget().uuid().setText(partition().fileSystem().uuid().isEmpty() ? i18nc("@item uuid", "(none)") : partition().fileSystem().uuid());

	setupFileSystemComboBox();

	 // don't do this before the file system combo box has been set up!
	dialogWidget().label().setText(newLabel().isEmpty() ? partition().fileSystem().label() : newLabel());
	dialogWidget().capacity().setText(Capacity::formatByteSize(partition().capacity()));

	if (Capacity(partition(), Capacity::Available).isValid())
	{
		const qint64 availPercent = (partition().fileSystem().length() - partition().fileSystem().sectorsUsed()) * 100 / partition().fileSystem().length();

		const QString availString = QStringLiteral("%1% - %2")
			.arg(availPercent)
			.arg(Capacity::formatByteSize(partition().available()));
		const QString usedString = QStringLiteral("%1% - %2")
			.arg(100 - availPercent)
			.arg(Capacity::formatByteSize(partition().used()));

		dialogWidget().available().setText(availString);
		dialogWidget().used().setText(usedString);
	}
	else
	{
		dialogWidget().available().setText(Capacity::invalidString());
		dialogWidget().used().setText(Capacity::invalidString());
	}

	dialogWidget().firstSector().setText(QLocale().toString(partition().firstSector()));
	dialogWidget().lastSector().setText(QLocale().toString(partition().lastSector()));
	dialogWidget().numSectors().setText(QLocale().toString(partition().length()));

	setupFlagsList();

	updateHideAndShow();

	setMinimumSize(dialogWidget().size());
	resize(dialogWidget().size());
}

void PartPropsDialog::setupFlagsList()
{
	int f = 1;
	QString s;
	while(!(s = PartitionTable::flagName(static_cast<PartitionTable::Flag>(f))).isEmpty())
	{
		if (partition().availableFlags() & f)
		{
			QListWidgetItem* item = new QListWidgetItem(s);
			dialogWidget().listFlags().addItem(item);
			item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			item->setData(Qt::UserRole, f);
			item->setCheckState((partition().activeFlags() & f) ? Qt::Checked : Qt::Unchecked);
		}

		f <<= 1;
	}
}

void PartPropsDialog::updateHideAndShow()
{
	// create a temporary fs for some checks
	const FileSystem* fs = FileSystemFactory::create(newFileSystemType(), -1, -1, -1, QString());

	if (fs == NULL || fs->supportSetLabel() == FileSystem::cmdSupportNone)
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
		dialogWidget().label().setReadOnly(isReadOnly());
		dialogWidget().noSetLabel().setVisible(false);
	}

	// when do we show the uuid?
	const bool showUuid =
			partition().state() != Partition::StateNew &&                           // not for new partitions
			!(fs == NULL || fs->supportGetUUID() == FileSystem::cmdSupportNone);       // not if the FS doesn't support it

	dialogWidget().showUuid(showUuid);

	delete fs;

	// when do we show available and used capacity?
	const bool showAvailableAndUsed =
			partition().state() != Partition::StateNew &&                           // not for new partitions
			!partition().roles().has(PartitionRole::Extended) &&                    // neither for extended
			!partition().roles().has(PartitionRole::Unallocated) &&                 // or for unallocated
			newFileSystemType() != FileSystem::Unformatted;                         // and not for unformatted file systems

	dialogWidget().showAvailable(showAvailableAndUsed);
	dialogWidget().showUsed(showAvailableAndUsed);

	// when do we show the file system combo box?
	const bool showFileSystem =
			!partition().roles().has(PartitionRole::Extended) &&                    // not for extended, they have no file system
			!partition().roles().has(PartitionRole::Unallocated);                   // and not for unallocated: no choice there

	dialogWidget().showFileSystem(showFileSystem);

	// when do we show the recreate file system check box?
	const bool showCheckRecreate =
			showFileSystem &&                                                       // only if we also show the file system
			partition().fileSystem().supportCreate() != FileSystem::cmdSupportNone &&  // and support creating this file system
			partition().fileSystem().type() != FileSystem::Unknown &&               // and not for unknown file systems
			partition().state() != Partition::StateNew;                             // or new partitions

	dialogWidget().showCheckRecreate(showCheckRecreate);

	// when do we show the list of partition flags?
	const bool showListFlags =
			partition().state() != Partition::StateNew &&                           // not for new partitions
			!partition().roles().has(PartitionRole::Unallocated);                   // and not for unallocated space

	dialogWidget().showListFlags(showListFlags);

	dialogWidget().checkRecreate().setEnabled(!isReadOnly());
	dialogWidget().listFlags().setEnabled(!isReadOnly());
	dialogWidget().fileSystem().setEnabled(!isReadOnly() && !forceRecreate());
}

void PartPropsDialog::setupConnections()
{
	connect(&dialogWidget().label(), SIGNAL(textEdited(const QString&)), SLOT(setDirty()));
	connect(&dialogWidget().fileSystem(), SIGNAL(currentIndexChanged(int)), SLOT(onFilesystemChanged(int)));
	connect(&dialogWidget().checkRecreate(), SIGNAL(stateChanged(int)), SLOT(onRecreate(int)));

	// We want to enable the OK-button whenever the user checks or unchecks a flag in the flag list.
	// But it seems Qt doesn't offer a foolproof way to detect if this has happened: The currentRow/ItemChanged
	// signal only means the _current_ item has changed, but not necessarily that it was checked/unchecked. And
	// itemClicked alone isn't enough either. We choose to rather enable the OK-button too often than too
	// seldom.
	connect(&dialogWidget().listFlags(), SIGNAL(itemClicked(QListWidgetItem*)), SLOT(setDirty()));
	connect(&dialogWidget().listFlags(), SIGNAL(currentRowChanged(int)), SLOT(setDirty()));

}

void PartPropsDialog::setDirty()
{
	okButton->setEnabled(true);
	okButton->setDefault(true);
}

void PartPropsDialog::setupFileSystemComboBox()
{
	dialogWidget().fileSystem().clear();
	QString selected;
	QStringList fsNames;

	foreach(const FileSystem* fs, FileSystemFactory::map())
		if (partition().fileSystem().type() == fs->type() || (fs->supportCreate() != FileSystem::cmdSupportNone && partition().capacity() >= fs->minCapacity() && partition().capacity() <= fs->maxCapacity()))
		{
			QString name = fs->name();

			if (partition().fileSystem().type() == fs->type())
				selected = name;

			// If the partition isn't extended, skip the extended FS
			if (fs->type() == FileSystem::Extended && !partition().roles().has(PartitionRole::Extended))
				continue;

			// The user cannot change the filesystem back to "unformatted" once a filesystem has been created.
			if (fs->type() == FileSystem::Unformatted)
			{
				// .. but if the file system is unknown to us, show the unformatted option as the currently selected one
				if (partition().fileSystem().type() == FileSystem::Unknown)
				{
					name = FileSystem::nameForType(FileSystem::Unformatted);
					selected = name;
				}
				else if (partition().fileSystem().type() != FileSystem::Unformatted && partition().state() != Partition::StateNew)
					continue;
			}

			fsNames.append(name);
		}

	qSort(fsNames.begin(), fsNames.end(), caseInsensitiveLessThan);

	foreach (const QString& fsName, fsNames)
		dialogWidget().fileSystem().addItem(createFileSystemColor(FileSystem::typeForName(fsName), 8), fsName);

	dialogWidget().fileSystem().setCurrentIndex(dialogWidget().fileSystem().findText(selected));

	const FileSystem* fs = FileSystemFactory::create(FileSystem::typeForName(dialogWidget().fileSystem().currentText()), -1, -1, -1, QString());
	dialogWidget().m_EditLabel->setMaxLength(fs->maxLabelLength());
}

void PartPropsDialog::updatePartitionFileSystem()
{
	FileSystem* fs = FileSystemFactory::create(newFileSystemType(), partition().firstSector(), partition().lastSector());
	partition().deleteFileSystem();
	partition().setFileSystem(fs);
	dialogWidget().partWidget().update();
}

void PartPropsDialog::onFilesystemChanged(int)
{
	if (partition().state() == Partition::StateNew || warnFileSystemChange() || KMessageBox::warningContinueCancel(this,
			xi18nc("@info", "<para><warning>You are about to lose all data on partition <filename>%1</filename>.<warning></para>"
				"<para>Changing the file system on a partition already on disk will erase all its contents. If you continue now and apply the resulting operation in the main window, all data on <filename>%1</filename> will unrecoverably be lost.</para>", partition().deviceNode()),
			xi18nc("@title:window", "Really Recreate <filename>%1</filename> with File System %2?", partition().deviceNode(), dialogWidget().fileSystem().currentText()),
			KGuiItem(i18nc("@action:button", "Change the File System"), QStringLiteral("arrow-right")),
			KGuiItem(i18nc("@action:button", "Do Not Change the File System"), QStringLiteral("dialog-cancel")), QStringLiteral("reallyChangeFileSystem")) == KMessageBox::Continue)
	{
		setDirty();
		updateHideAndShow();
		setWarnFileSystemChange();
		updatePartitionFileSystem();

		const FileSystem* fs = FileSystemFactory::create(FileSystem::typeForName(dialogWidget().fileSystem().currentText()), -1, -1, -1, QString());
		dialogWidget().m_EditLabel->setMaxLength(fs->maxLabelLength());
	}
	else
	{
		dialogWidget().fileSystem().disconnect(this);
		setupFileSystemComboBox();
		connect(&dialogWidget().fileSystem(), SIGNAL(currentIndexChanged(int)), SLOT(onFilesystemChanged(int)));
	}
}

void PartPropsDialog::onRecreate(int state)
{
	if (state == Qt::Checked && (warnFileSystemChange() || KMessageBox::warningContinueCancel(this,
			xi18nc("@info", "<para><warning>You are about to lose all data on partition <filename>%1</filename>.</warning></para>"
				"<para>Recreating a file system will erase all its contents. If you continue now and apply the resulting operation in the main window, all data on <filesystem>%1</filesyste> will unrecoverably be lost.</p>", partition().deviceNode()),
			xi18nc("@title:window", "Really Recreate File System on <filename>%1</filename>?", partition().deviceNode()),
			KGuiItem(i18nc("@action:button", "Recreate the File System"), QStringLiteral("arrow-right")),
			KGuiItem(i18nc("@action:button", "Do Not Recreate the File System"), QStringLiteral("dialog-cancel")), QStringLiteral("reallyRecreateFileSystem")) == KMessageBox::Continue))
	{
		setDirty();
		setWarnFileSystemChange();
		setForceRecreate(true);
		dialogWidget().fileSystem().setCurrentIndex(dialogWidget().fileSystem().findText(partition().fileSystem().name()));
		dialogWidget().fileSystem().setEnabled(false);
		updateHideAndShow();
		updatePartitionFileSystem();
	}
	else
	{
		setForceRecreate(false);
		dialogWidget().checkRecreate().setCheckState(Qt::Unchecked);
		dialogWidget().fileSystem().setEnabled(true);
		updateHideAndShow();
	}
}
