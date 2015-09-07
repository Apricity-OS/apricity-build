/***************************************************************************
 *   Copyright (C) 2008,2009,2010,2012 by Volker Lanz <vl@fidra.de>        *
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

#include "gui/mainwindow.h"
#include "gui/infopane.h"
#include "gui/applyprogressdialog.h"
#include "gui/scanprogressdialog.h"
#include "gui/createpartitiontabledialog.h"
#include "gui/filesystemsupportdialog.h"
#include "gui/devicepropsdialog.h"
#include "gui/smartdialog.h"

#include "config/configureoptionsdialog.h"

#include "backend/corebackendmanager.h"
#include "backend/corebackend.h"

#include "core/device.h"
#include "core/partition.h"
#include "core/smartstatus.h"

#include "ops/operation.h"
#include "ops/createpartitiontableoperation.h"
#include "ops/resizeoperation.h"
#include "ops/copyoperation.h"
#include "ops/deleteoperation.h"
#include "ops/newoperation.h"
#include "ops/backupoperation.h"
#include "ops/restoreoperation.h"
#include "ops/checkoperation.h"

#include "fs/filesystem.h"
#include "fs/filesystemfactory.h"

#include "util/helpers.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QMenu>
#include <QPointer>
#include <QReadLocker>
#include <QStatusBar>
#include <QTemporaryFile>
#include <QTextStream>

#include <KXmlGui/KActionCollection>
#include <KMessageBox>
#include <KAboutData>
#include <KIconThemes/KIconLoader>
#include <KLocalizedString>
#include <KXmlGui/KXMLGUIFactory>
#include <KJobUiDelegate>
#include <KIO/CopyJob>
#include <KIO/Job>
#include <KJobWidgets/KJobWidgets>

#include <config.h>

#include <unistd.h>

/** Creates a new MainWindow instance.
	@param parent the parent widget
*/
MainWindow::MainWindow(QWidget* parent) :
	KXmlGuiWindow(parent),
	Ui::MainWindowBase(),
	m_OperationStack(new OperationStack(this)),
	m_OperationRunner(new OperationRunner(this, operationStack())),
	m_DeviceScanner(new DeviceScanner(this, operationStack())),
	m_ApplyProgressDialog(new ApplyProgressDialog(this, operationRunner())),
	m_ScanProgressDialog(new ScanProgressDialog(this)),
	m_StatusText(new QLabel(this))
{
	setupObjectNames();
	setupUi(this);
	init();
}

void MainWindow::setupObjectNames()
{
	m_OperationStack->setObjectName(QStringLiteral("m_OperationStack"));
	m_OperationRunner->setObjectName(QStringLiteral("m_OperationRunner"));
	m_DeviceScanner->setObjectName(QStringLiteral("m_DeviceScanner"));
	m_ApplyProgressDialog->setObjectName(QStringLiteral("m_ApplyProgressDialog"));
	m_ScanProgressDialog->setObjectName(QStringLiteral("m_ScanProgressDialog"));
}

void MainWindow::init()
{
	treeLog().init();

	connect(GlobalLog::instance(), SIGNAL(newMessage(Log::Level, const QString&)), &treeLog(), SLOT(onNewLogMessage(Log::Level, const QString&)));

	setupActions();
	setupStatusBar();
	setupConnections();

	listDevices().setActionCollection(actionCollection());
	listOperations().setActionCollection(actionCollection());
	pmWidget().init(&operationStack());

	setupGUI();

	loadConfig();

	scanDevices();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (applyProgressDialog().isVisible())
	{
		event->ignore();
		return;
	}

	if (operationStack().size() > 0)
	{
		if (KMessageBox::warningContinueCancel(this,
			xi18ncp("@info", "<para>Do you really want to quit the application?</para><para>There is still an operation pending.</para>",
		"<para>Do you really want to quit the application?</para><para>There are still %1 operations pending.</para>", operationStack().size()),
			i18nc("@title:window", "Discard Pending Operations and Quit?"),
			KGuiItem(xi18nc("@action:button", "Quit <application>%1</application>", QGuiApplication::applicationDisplayName()), QStringLiteral("arrow-right")),
			KStandardGuiItem::cancel(), QStringLiteral("reallyQuit")) == KMessageBox::Cancel)
		{
			event->ignore();
			return;
		}
	}

	saveConfig();

	KXmlGuiWindow::closeEvent(event);
}

void MainWindow::changeEvent(QEvent* event)
{
	if ((event->type() == QEvent::ActivationChange || event->type() == QEvent::WindowStateChange) && event->spontaneous() && isActiveWindow())
	{
		QWidget* w = NULL;

		if (applyProgressDialog().isVisible())
			w = &applyProgressDialog();
		else if (scanProgressDialog().isVisible())
			w = &scanProgressDialog();

		if (w != NULL)
		{
			w->activateWindow();
			w->raise();
			w->setFocus();
		}
	}

	KXmlGuiWindow::changeEvent(event);
}

void MainWindow::setupActions()
{
	// File actions
	KStandardAction::quit(this, SLOT(close()), actionCollection());

	// Edit actions
	QAction* undoOperation = actionCollection()->addAction(QStringLiteral("undoOperation"), this, SLOT(onUndoOperation()));
	undoOperation->setEnabled(false);
	undoOperation->setText(i18nc("@action:inmenu", "Undo"));
	undoOperation->setToolTip(i18nc("@info:tooltip", "Undo the last operation"));
	undoOperation->setStatusTip(i18nc("@info:status", "Remove the last operation from the list."));
	undoOperation->setShortcut(Qt::CTRL | Qt::Key_Z);
	undoOperation->setIcon(QIcon(KIconLoader().loadIcon(QStringLiteral("edit-undo"), KIconLoader::Toolbar)));

	QAction* clearAllOperations = actionCollection()->addAction(QStringLiteral("clearAllOperations"), this, SLOT(onClearAllOperations()));
	clearAllOperations->setEnabled(false);
	clearAllOperations->setText(i18nc("@action:inmenu clear the list of operations", "Clear"));
	clearAllOperations->setToolTip(i18nc("@info:tooltip", "Clear all operations"));
	clearAllOperations->setStatusTip(i18nc("@info:status", "Empty the list of pending operations."));
	clearAllOperations->setIcon(QIcon(KIconLoader().loadIcon(QStringLiteral("dialog-cancel"), KIconLoader::Toolbar)));

	QAction* applyAllOperations = actionCollection()->addAction(QStringLiteral("applyAllOperations"), this, SLOT(onApplyAllOperations()));
	applyAllOperations->setEnabled(false);
	applyAllOperations->setText(i18nc("@action:inmenu apply all operations", "Apply"));
	applyAllOperations->setToolTip(i18nc("@info:tooltip", "Apply all operations"));
	applyAllOperations->setStatusTip(i18nc("@info:status", "Apply the pending operations in the list."));
	applyAllOperations->setIcon(QIcon(KIconLoader().loadIcon(QStringLiteral("dialog-ok-apply"), KIconLoader::Toolbar)));

	// Device actions
	QAction* refreshDevices = actionCollection()->addAction(QStringLiteral("refreshDevices"),  this, SLOT(onRefreshDevices()));
	refreshDevices->setText(i18nc("@action:inmenu refresh list of devices", "Refresh Devices"));
	refreshDevices->setToolTip(i18nc("@info:tooltip", "Refresh all devices"));
	refreshDevices->setStatusTip(i18nc("@info:status", "Renew the devices list."));
	refreshDevices->setShortcut(Qt::Key_F5);
	refreshDevices->setIcon(QIcon(KIconLoader().loadIcon(QStringLiteral("view-refresh"), KIconLoader::Toolbar)));

	QAction* createNewPartitionTable = actionCollection()->addAction(QStringLiteral("createNewPartitionTable"), this, SLOT(onCreateNewPartitionTable()));
	createNewPartitionTable->setEnabled(false);
	createNewPartitionTable->setText(i18nc("@action:inmenu", "New Partition Table"));
	createNewPartitionTable->setToolTip(i18nc("@info:tooltip", "Create a new partition table"));
	createNewPartitionTable->setStatusTip(i18nc("@info:status", "Create a new and empty partition table on a device."));
	createNewPartitionTable->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_N);
	createNewPartitionTable->setIcon(QIcon(KIconLoader().loadIcon(QStringLiteral("edit-clear"), KIconLoader::Toolbar)));

	QAction* exportPartitionTable = actionCollection()->addAction(QStringLiteral("exportPartitionTable"), this, SLOT(onExportPartitionTable()));
	exportPartitionTable->setEnabled(false);
	exportPartitionTable->setText(i18nc("@action:inmenu", "Export Partition Table"));
	exportPartitionTable->setToolTip(i18nc("@info:tooltip", "Export a partition table"));
	exportPartitionTable->setStatusTip(i18nc("@info:status", "Export the device's partition table to a text file."));
	exportPartitionTable->setIcon(QIcon(KIconLoader().loadIcon(QStringLiteral("document-export"), KIconLoader::Toolbar)));

	QAction* importPartitionTable = actionCollection()->addAction(QStringLiteral("importPartitionTable"), this, SLOT(onImportPartitionTable()));
	importPartitionTable->setEnabled(false);
	importPartitionTable->setText(i18nc("@action:inmenu", "Import Partition Table"));
	importPartitionTable->setToolTip(i18nc("@info:tooltip", "Import a partition table"));
	importPartitionTable->setStatusTip(i18nc("@info:status", "Import a partition table from a text file."));
	importPartitionTable->setIcon(QIcon(KIconLoader().loadIcon(QStringLiteral("document-import"), KIconLoader::Toolbar)));

	QAction* smartStatusDevice = actionCollection()->addAction(QStringLiteral("smartStatusDevice"), this, SLOT(onSmartStatusDevice()));
	smartStatusDevice->setEnabled(false);
	smartStatusDevice->setText(i18nc("@action:inmenu", "SMART Status"));
	smartStatusDevice->setToolTip(i18nc("@info:tooltip", "Show SMART status"));
	smartStatusDevice->setStatusTip(i18nc("@info:status", "Show the device's SMART status if supported"));

	QAction* propertiesDevice = actionCollection()->addAction(QStringLiteral("propertiesDevice"), this, SLOT(onPropertiesDevice()));
	propertiesDevice->setEnabled(false);
	propertiesDevice->setText(i18nc("@action:inmenu", "Properties"));
	propertiesDevice->setToolTip(i18nc("@info:tooltip", "Show device properties dialog"));
	propertiesDevice->setStatusTip(i18nc("@info:status", "View and modify device properties"));
	propertiesDevice->setIcon(QIcon(KIconLoader().loadIcon(QStringLiteral("document-properties"), KIconLoader::Toolbar)));

	// Partition actions
	QAction* newPartition = actionCollection()->addAction(QStringLiteral("newPartition"), &pmWidget(), SLOT(onNewPartition()));
	newPartition->setEnabled(false);
	newPartition->setText(i18nc("@action:inmenu create a new partition", "New"));
	newPartition->setToolTip(i18nc("@info:tooltip", "New partition"));
	newPartition->setStatusTip(i18nc("@info:status", "Create a new partition."));
	newPartition->setShortcut(Qt::CTRL | Qt::Key_N);
	newPartition->setIcon(QIcon(KIconLoader().loadIcon(QStringLiteral("document-new"), KIconLoader::Toolbar)));

	QAction* resizePartition = actionCollection()->addAction(QStringLiteral("resizePartition"), &pmWidget(), SLOT(onResizePartition()));
	resizePartition->setEnabled(false);
	resizePartition->setText(i18nc("@action:inmenu", "Resize/Move"));
	resizePartition->setToolTip(i18nc("@info:tooltip", "Resize or move partition"));
	resizePartition->setStatusTip(i18nc("@info:status", "Shrink, grow or move an existing partition."));
	resizePartition->setShortcut(Qt::CTRL | Qt::Key_R);
	resizePartition->setIcon(QIcon(KIconLoader().loadIcon(QStringLiteral("arrow-right-double"), KIconLoader::Toolbar)));

	QAction* deletePartition = actionCollection()->addAction(QStringLiteral("deletePartition"), &pmWidget(), SLOT(onDeletePartition()));
	deletePartition->setEnabled(false);
	deletePartition->setText(i18nc("@action:inmenu", "Delete"));
	deletePartition->setToolTip(i18nc("@info:tooltip", "Delete partition"));
	deletePartition->setStatusTip(i18nc("@info:status", "Delete a partition."));
	deletePartition->setShortcut(Qt::Key_Delete);
	deletePartition->setIcon(QIcon(KIconLoader().loadIcon(QStringLiteral("edit-delete"), KIconLoader::Toolbar)));

	QAction* shredPartition = actionCollection()->addAction(QStringLiteral("shredPartition"), &pmWidget(), SLOT(onShredPartition()));
	shredPartition->setEnabled(false);
	shredPartition->setText(i18nc("@action:inmenu", "Shred"));
	shredPartition->setToolTip(i18nc("@info:tooltip", "Shred partition"));
	shredPartition->setStatusTip(i18nc("@info:status", "Shred a partition so that its contents cannot be restored."));
	shredPartition->setShortcut(Qt::SHIFT | Qt::Key_Delete);
	shredPartition->setIcon(QIcon(KIconLoader().loadIcon(QStringLiteral("edit-delete-shred"), KIconLoader::Toolbar)));

	QAction* copyPartition = actionCollection()->addAction(QStringLiteral("copyPartition"), &pmWidget(), SLOT(onCopyPartition()));
	copyPartition->setEnabled(false);
	copyPartition->setText(i18nc("@action:inmenu", "Copy"));
	copyPartition->setToolTip(i18nc("@info:tooltip", "Copy partition"));
	copyPartition->setStatusTip(i18nc("@info:status", "Copy an existing partition."));
	copyPartition->setShortcut(Qt::CTRL | Qt::Key_C);
	copyPartition->setIcon(QIcon(KIconLoader().loadIcon(QStringLiteral("edit-copy"), KIconLoader::Toolbar)));

	QAction* pastePartition = actionCollection()->addAction(QStringLiteral("pastePartition"), &pmWidget(), SLOT(onPastePartition()));
	pastePartition->setEnabled(false);
	pastePartition->setText(i18nc("@action:inmenu", "Paste"));
	pastePartition->setToolTip(i18nc("@info:tooltip", "Paste partition"));
	pastePartition->setStatusTip(i18nc("@info:status", "Paste a copied partition."));
	pastePartition->setShortcut(Qt::CTRL | Qt::Key_V);
	pastePartition->setIcon(QIcon(KIconLoader().loadIcon(QStringLiteral("edit-paste"), KIconLoader::Toolbar)));

	QAction* editMountPoint = actionCollection()->addAction(QStringLiteral("editMountPoint"), &pmWidget(), SLOT(onEditMountPoint()));
	editMountPoint->setEnabled(false);
	editMountPoint->setText(i18nc("@action:inmenu", "Edit Mount Point"));
	editMountPoint->setToolTip(i18nc("@info:tooltip", "Edit mount point"));
	editMountPoint->setStatusTip(i18nc("@info:status", "Edit a partition's mount point and options."));

	QAction* mountPartition = actionCollection()->addAction(QStringLiteral("mountPartition"), &pmWidget(), SLOT(onMountPartition()));
	mountPartition->setEnabled(false);
	mountPartition->setText(i18nc("@action:inmenu", "Mount"));
	mountPartition->setToolTip(i18nc("@info:tooltip", "Mount or unmount partition"));
	mountPartition->setStatusTip(i18nc("@info:status", "Mount or unmount a partition."));

	QAction* checkPartition = actionCollection()->addAction(QStringLiteral("checkPartition"), &pmWidget(), SLOT(onCheckPartition()));
	checkPartition->setEnabled(false);
	checkPartition->setText(i18nc("@action:inmenu", "Check"));
	checkPartition->setToolTip(i18nc("@info:tooltip", "Check partition"));
	checkPartition->setStatusTip(i18nc("@info:status", "Check a filesystem on a partition for errors."));
	checkPartition->setIcon(QIcon(KIconLoader().loadIcon(QStringLiteral("flag"), KIconLoader::Toolbar)));

	QAction* propertiesPartition = actionCollection()->addAction(QStringLiteral("propertiesPartition"), &pmWidget(), SLOT(onPropertiesPartition()));
	propertiesPartition->setEnabled(false);
	propertiesPartition->setText(i18nc("@action:inmenu", "Properties"));
	propertiesPartition->setToolTip(i18nc("@info:tooltip", "Show partition properties dialog"));
	propertiesPartition->setStatusTip(i18nc("@info:status", "View and modify partition properties (label, partition flags, etc.)"));
	propertiesPartition->setIcon(QIcon(KIconLoader().loadIcon(QStringLiteral("document-properties"), KIconLoader::Toolbar)));

	QAction* backup = actionCollection()->addAction(QStringLiteral("backupPartition"), &pmWidget(), SLOT(onBackupPartition()));
	backup->setEnabled(false);
	backup->setText(i18nc("@action:inmenu", "Backup"));
	backup->setToolTip(i18nc("@info:tooltip", "Backup partition"));
	backup->setStatusTip(i18nc("@info:status", "Backup a partition to an image file."));
	backup->setIcon(QIcon(KIconLoader().loadIcon(QStringLiteral("document-export"), KIconLoader::Toolbar)));

	QAction* restore = actionCollection()->addAction(QStringLiteral("restorePartition"), &pmWidget(), SLOT(onRestorePartition()));
	restore->setEnabled(false);
	restore->setText(i18nc("@action:inmenu", "Restore"));
	restore->setToolTip(i18nc("@info:tooltip", "Restore partition"));
	restore->setStatusTip(i18nc("@info:status", "Restore a partition from an image file."));
	restore->setIcon(QIcon(KIconLoader().loadIcon(QStringLiteral("document-import"), KIconLoader::Toolbar)));

	// View actions
	QAction* fileSystemSupport = actionCollection()->addAction(QStringLiteral("fileSystemSupport"), this, SLOT(onFileSystemSupport()));
	fileSystemSupport->setText(i18nc("@action:inmenu", "File System Support"));
	fileSystemSupport->setToolTip(i18nc("@info:tooltip", "View file system support information"));
	fileSystemSupport->setStatusTip(i18nc("@info:status", "Show information about supported file systems."));

	actionCollection()->addAction(QStringLiteral("toggleDockDevices"), dockDevices().toggleViewAction());
	actionCollection()->addAction(QStringLiteral("toggleDockOperations"), dockOperations().toggleViewAction());
	actionCollection()->addAction(QStringLiteral("toggleDockInformation"), dockInformation().toggleViewAction());
	actionCollection()->addAction(QStringLiteral("toggleDockLog"), dockLog().toggleViewAction());

	// Settings Actions
	KStandardAction::preferences(this, SLOT(onConfigureOptions()), actionCollection());

	// Log Actions
	QAction* clearLog = actionCollection()->addAction(QStringLiteral("clearLog"), &treeLog(), SLOT(onClearLog()));
	clearLog->setText(i18nc("@action:inmenu", "Clear Log"));
	clearLog->setToolTip(i18nc("@info:tooltip", "Clear the log output"));
	clearLog->setStatusTip(i18nc("@info:status", "Clear the log output panel."));
	clearLog->setIcon(QIcon(KIconLoader().loadIcon(QStringLiteral("edit-clear-list"), KIconLoader::Toolbar)));

	QAction* saveLog = actionCollection()->addAction(QStringLiteral("saveLog"), &treeLog(), SLOT(onSaveLog()));
	saveLog->setText(i18nc("@action:inmenu", "Save Log"));
	saveLog->setToolTip(i18nc("@info:tooltip", "Save the log output"));
	saveLog->setStatusTip(i18nc("@info:status", "Save the log output to a file."));
	saveLog->setIcon(QIcon(KIconLoader().loadIcon(QStringLiteral("document-save"), KIconLoader::Toolbar)));
}

void MainWindow::setupConnections()
{
	connect(&listDevices(), SIGNAL(selectionChanged(const QString&)), &pmWidget(), SLOT(setSelectedDevice(const QString&)));
	connect(&listDevices(), SIGNAL(deviceDoubleClicked(const QString&)), SLOT(onPropertiesDevice(const QString&)));
}

void MainWindow::setupStatusBar()
{
	statusBar()->addWidget(&statusText());
}

void MainWindow::loadConfig()
{
	if (Config::firstRun())
	{
		dockLog().setVisible(false);
		dockInformation().setVisible(false);
	}
}

void MainWindow::saveConfig() const
{
	Config::setFirstRun(false);
	Config::self()->writeConfig();
}

void MainWindow::enableActions()
{
	actionCollection()->action(QStringLiteral("createNewPartitionTable"))->setEnabled(CreatePartitionTableOperation::canCreate(pmWidget().selectedDevice()));
	actionCollection()->action(QStringLiteral("exportPartitionTable"))->setEnabled(pmWidget().selectedDevice() && pmWidget().selectedDevice()->partitionTable() && operationStack().size() == 0);
	actionCollection()->action(QStringLiteral("importPartitionTable"))->setEnabled(CreatePartitionTableOperation::canCreate(pmWidget().selectedDevice()));
	actionCollection()->action(QStringLiteral("smartStatusDevice"))->setEnabled(pmWidget().selectedDevice() != NULL && pmWidget().selectedDevice()->smartStatus().isValid());
	actionCollection()->action(QStringLiteral("propertiesDevice"))->setEnabled(pmWidget().selectedDevice() != NULL);

	actionCollection()->action(QStringLiteral("undoOperation"))->setEnabled(operationStack().size() > 0);
	actionCollection()->action(QStringLiteral("clearAllOperations"))->setEnabled(operationStack().size() > 0);
	actionCollection()->action(QStringLiteral("applyAllOperations"))->setEnabled(operationStack().size() > 0 && (geteuid() == 0 || Config::allowApplyOperationsAsNonRoot()));

	const bool readOnly = pmWidget().selectedDevice() == NULL ||
			pmWidget().selectedDevice()->partitionTable() == NULL ||
			pmWidget().selectedDevice()->partitionTable()->isReadOnly();

	const Partition* part = pmWidget().selectedPartition();

	actionCollection()->action(QStringLiteral("newPartition"))->setEnabled(!readOnly && NewOperation::canCreateNew(part));
	const bool canResize = ResizeOperation::canGrow(part) || ResizeOperation::canShrink(part) || ResizeOperation::canMove(part);
	actionCollection()->action(QStringLiteral("resizePartition"))->setEnabled(!readOnly && canResize);
	actionCollection()->action(QStringLiteral("copyPartition"))->setEnabled(CopyOperation::canCopy(part));
	actionCollection()->action(QStringLiteral("deletePartition"))->setEnabled(!readOnly && DeleteOperation::canDelete(part));
	actionCollection()->action(QStringLiteral("shredPartition"))->setEnabled(!readOnly && DeleteOperation::canDelete(part));
	actionCollection()->action(QStringLiteral("pastePartition"))->setEnabled(!readOnly && CopyOperation::canPaste(part, pmWidget().clipboardPartition()));
	actionCollection()->action(QStringLiteral("propertiesPartition"))->setEnabled(part != NULL);

	actionCollection()->action(QStringLiteral("editMountPoint"))->setEnabled(part && !part->isMounted());
	actionCollection()->action(QStringLiteral("mountPartition"))->setEnabled(part && (part->canMount() || part->canUnmount()));

	if (part != NULL)
		actionCollection()->action(QStringLiteral("mountPartition"))->setText(part->isMounted() ? part->fileSystem().unmountTitle() : part->fileSystem().mountTitle());

	actionCollection()->action(QStringLiteral("checkPartition"))->setEnabled(!readOnly && CheckOperation::canCheck(part));

	actionCollection()->action(QStringLiteral("backupPartition"))->setEnabled(BackupOperation::canBackup(part));
	actionCollection()->action(QStringLiteral("restorePartition"))->setEnabled(RestoreOperation::canRestore(part));
}

void MainWindow::on_m_ApplyProgressDialog_finished()
{
	scanDevices();
}

void MainWindow::on_m_OperationStack_operationsChanged()
{
	listOperations().updateOperations(operationStack().operations());
	pmWidget().updatePartitions();
	enableActions();

	// this will make sure that the info pane gets updated
	on_m_PartitionManagerWidget_selectedPartitionChanged(pmWidget().selectedPartition());

	statusText().setText(i18ncp("@info:status", "One pending operation", "%1 pending operations", operationStack().size()));
}

void MainWindow::on_m_OperationStack_devicesChanged()
{
	QReadLocker lockDevices(&operationStack().lock());

	listDevices().updateDevices(operationStack().previewDevices());

	if (pmWidget().selectedDevice())
		infoPane().showDevice(dockWidgetArea(&dockInformation()), *pmWidget().selectedDevice());
	else
		infoPane().clear();

	updateWindowTitle();
}

void MainWindow::on_m_DockInformation_dockLocationChanged(Qt::DockWidgetArea)
{
	on_m_PartitionManagerWidget_selectedPartitionChanged(pmWidget().selectedPartition());
}

void MainWindow::updateWindowTitle()
{
	QString title;

	if (pmWidget().selectedDevice())
		title = pmWidget().selectedDevice()->deviceNode();

	setWindowTitle(title);
}

void MainWindow::on_m_ListOperations_contextMenuRequested(const QPoint& pos)
{
	QMenu* menu = static_cast<QMenu*>(guiFactory()->container(QStringLiteral("edit"), this));

	if (menu)
		menu->exec(pos);
}

void MainWindow::on_m_TreeLog_contextMenuRequested(const QPoint& pos)
{
	QMenu* menu = static_cast<QMenu*>(guiFactory()->container(QStringLiteral("log"), this));

	if (menu)
		menu->exec(pos);
}

void MainWindow::on_m_ListDevices_contextMenuRequested(const QPoint& pos)
{
	QMenu* menu = static_cast<QMenu*>(guiFactory()->container(QStringLiteral("device"), this));

	if (menu)
		menu->exec(pos);
}

void MainWindow::on_m_PartitionManagerWidget_contextMenuRequested(const QPoint& pos)
{
	QMenu* menu = NULL;

	if (pmWidget().selectedPartition() == NULL)
	{
		if (pmWidget().selectedDevice() != NULL)
			menu = static_cast<QMenu*>(guiFactory()->container(QStringLiteral("device"), this));
	}
	else
		menu = static_cast<QMenu*>(guiFactory()->container(QStringLiteral("partition"), this));

	if (menu)
		menu->exec(pos);
}

void MainWindow::on_m_PartitionManagerWidget_deviceDoubleClicked(const Device*)
{
	actionCollection()->action(QStringLiteral("propertiesDevice"))->trigger();
}

void MainWindow::on_m_PartitionManagerWidget_partitionDoubleClicked(const Partition*)
{
	actionCollection()->action(QStringLiteral("propertiesPartition"))->trigger();
}

void MainWindow::on_m_PartitionManagerWidget_selectedPartitionChanged(const Partition* p)
{
	if (p)
		infoPane().showPartition(dockWidgetArea(&dockInformation()), *p);
	else if (pmWidget().selectedDevice())
		infoPane().showDevice(dockWidgetArea(&dockInformation()), *pmWidget().selectedDevice());
	else
		infoPane().clear();

	updateWindowTitle();
	enableActions();
}

void MainWindow::scanDevices()
{
// 	FIXME: port KF5
// 	Log(Log::information) << i18nc("@info/plain", "Using backend plugin: %1 (%2)",
// 			CoreBackendManager::self()->backend()->about().displayName(),
// 			CoreBackendManager::self()->backend()->about().version());

	Log() << i18nc("@info/plain", "Scanning devices...");

	// remember the currently selected device's node
	setSavedSelectedDeviceNode(pmWidget().selectedDevice() ?  pmWidget().selectedDevice()->deviceNode() : QString());

	pmWidget().clear();

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	scanProgressDialog().setEnabled(true);
	scanProgressDialog().show();

	deviceScanner().start();
}

void MainWindow::on_m_DeviceScanner_progress(const QString& device_node, int percent)
{
	scanProgressDialog().setProgress(percent);
	scanProgressDialog().setDeviceName(device_node);
}

void MainWindow::on_m_DeviceScanner_finished()
{
	QReadLocker lockDevices(&operationStack().lock());

	scanProgressDialog().setProgress(100);

	if (!operationStack().previewDevices().isEmpty())
		pmWidget().setSelectedDevice(operationStack().previewDevices()[0]);

	pmWidget().updatePartitions();

	Log() << i18nc("@info/plain", "Scan finished.");
	QApplication::restoreOverrideCursor();

	// try to set the seleted device, either from the saved one or just select the
	// first device
	if (!listDevices().setSelectedDevice(savedSelectedDeviceNode()) && !operationStack().previewDevices().isEmpty())
		listDevices().setSelectedDevice(operationStack().previewDevices()[0]->deviceNode());

	updateSeletedDeviceMenu();
	checkFileSystemSupport();
}

void MainWindow::updateSeletedDeviceMenu()
{
	QMenu* devicesMenu = static_cast<QMenu*>(guiFactory()->container(QStringLiteral("selectedDevice"), this));
	devicesMenu->clear();

	devicesMenu->setEnabled(!operationStack().previewDevices().isEmpty());

	foreach(const Device* d, operationStack().previewDevices())
	{
		QAction* action = new QAction(d->prettyName(), devicesMenu);
		action->setCheckable(true);
		action->setChecked(d->deviceNode() == pmWidget().selectedDevice()->deviceNode());
		action->setData(d->deviceNode());
		connect(action, SIGNAL(triggered(bool)), SLOT(onSelectedDeviceMenuTriggered(bool)));
		devicesMenu->addAction(action);
	}
}

void MainWindow::onSelectedDeviceMenuTriggered(bool)
{
	QAction* action = qobject_cast<QAction*>(sender());
	QMenu* devicesMenu = static_cast<QMenu*>(guiFactory()->container(QStringLiteral("selectedDevice"), this));

	if (action == NULL || action->parent() != devicesMenu)
		return;

	foreach (QAction* entry, devicesMenu->findChildren<QAction*>())
		entry->setChecked(entry == action);

	listDevices().setSelectedDevice(action->data().toString());
}

void MainWindow::on_m_ListDevices_selectionChanged(const QString& device_node)
{
	QMenu* devicesMenu = static_cast<QMenu*>(guiFactory()->container(QStringLiteral("selectedDevice"), this));

	foreach (QAction* entry, devicesMenu->findChildren<QAction*>())
		entry->setChecked(entry->data().toString() == device_node);
}

void MainWindow::onRefreshDevices()
{
	if (operationStack().size() == 0 || KMessageBox::warningContinueCancel(this,
		xi18nc("@info",
			"<para>Do you really want to rescan the devices?</para>"
			"<para><warning>This will also clear the list of pending operations.</warning></para>"),
		i18nc("@title:window", "Really Rescan the Devices?"),
		KGuiItem(i18nc("@action:button", "Rescan Devices"), QStringLiteral("arrow-right")),
		KStandardGuiItem::cancel(), QStringLiteral("reallyRescanDevices")) == KMessageBox::Continue)
	{
		scanDevices();
	}
}

void MainWindow::onApplyAllOperations()
{
	QStringList opList;

	foreach (const Operation* op, operationStack().operations())
		opList.append(op->description());

	if (KMessageBox::warningContinueCancelList(this,
		xi18nc("@info",
			"<para>Do you really want to apply the pending operations listed below?</para>"
			"<para><warning>This will permanently modify your disks.</warning></para>"),
		opList, i18nc("@title:window", "Apply Pending Operations?"),
		KGuiItem(i18nc("@action:button", "Apply Pending Operations"), QStringLiteral("arrow-right")),
		KStandardGuiItem::cancel()) == KMessageBox::Continue)
	{
		Log() << i18nc("@info/plain", "Applying operations...");

		applyProgressDialog().show();

		operationRunner().setReport(&applyProgressDialog().report());

		// Undo all operations so the runner has a defined starting point
		for (int i = operationStack().operations().size() - 1; i >= 0; i--)
		{
			operationStack().operations()[i]->undo();
			operationStack().operations()[i]->setStatus(Operation::StatusNone);
		}

		pmWidget().updatePartitions();

		operationRunner().start();
	}
}

void MainWindow::onUndoOperation()
{
	Q_ASSERT(operationStack().size() > 0);

	if (operationStack().size() == 0)
		return;

	Log() << i18nc("@info/plain", "Undoing operation: %1", operationStack().operations().last()->description());
	operationStack().pop();

	// it's possible the undo killed the partition in the clipboard. if there's a partition in the clipboard, try
	// to find a device for it (OperationStack::findDeviceForPartition() only compares pointers, so an invalid
	// pointer is not a problem). if no device is found, the pointer must be dangling, so clear the clipboard.
	if (pmWidget().clipboardPartition() != NULL && operationStack().findDeviceForPartition(pmWidget().clipboardPartition()) == NULL)
		pmWidget().setClipboardPartition(NULL);

	pmWidget().updatePartitions();
	enableActions();
}

void MainWindow::onClearAllOperations()
{
	if (KMessageBox::warningContinueCancel(this,
		i18nc("@info", "Do you really want to clear the list of pending operations?"),
		i18nc("@title:window", "Clear Pending Operations?"),
		KGuiItem(i18nc("@action:button", "Clear Pending Operations"), QStringLiteral("arrow-right")),
		KStandardGuiItem::cancel(), QStringLiteral("reallyClearPendingOperations")) == KMessageBox::Continue)
	{
		Log() << i18nc("@info/plain", "Clearing the list of pending operations.");
		operationStack().clearOperations();

		pmWidget().updatePartitions();
		enableActions();
	}
}

void MainWindow::onCreateNewPartitionTable()
{
	Q_ASSERT(pmWidget().selectedDevice());

	if (pmWidget().selectedDevice() == NULL)
	{
		qWarning() << "selected device is null.";
		return;
	}

	QPointer<CreatePartitionTableDialog> dlg = new CreatePartitionTableDialog(this, *pmWidget().selectedDevice());

	if (dlg->exec() == QDialog::Accepted)
		operationStack().push(new CreatePartitionTableOperation(*pmWidget().selectedDevice(), dlg->type()));

	delete dlg;
}

void MainWindow::onImportPartitionTable()
{
	Q_ASSERT(pmWidget().selectedDevice());

	const QUrl url = QFileDialog::getOpenFileUrl(this, QStringLiteral("kfiledialog://importPartitionTable"));

	if (url.isEmpty())
		return;

	QString fileName;
	KIO::FileCopyJob *job = KIO::file_copy(url, QUrl::fromLocalFile(fileName));
	KJobWidgets::setWindow(job, this);
	job->exec();
	if ( job->error() )
	{
		KMessageBox::error(this, xi18nc("@info", "Could not open input file <filename>%1</filename> for import: %2", url.fileName(), job->errorString()), i18nc("@title:window", "Error Importing Partition Table"));
		return;
	}

	QFile file(fileName);

	if (!file.open(QFile::ReadOnly))
	{
		KMessageBox::error(this, xi18nc("@info", "Could not open temporary file <filename>%1</filename> while trying to import from <filename>%2</filename>.", fileName, url.fileName()), i18nc("@title:window", "Error Importing Partition Table"));
		return;
	}

	Device& device = *pmWidget().selectedDevice();

	QByteArray line;
	QRegExp rxPartition(QStringLiteral("(\\d+);(\\d+);(\\d+);(\\w+);(\\w+);(\"\\w*\");(\"[^\"]*\")"));
	QRegExp rxType(QStringLiteral("type:\\s\"(.+)\""));
	QRegExp rxAlign(QStringLiteral("align:\\s\"(cylinder|sector)\""));
	QRegExp rxMagic(QStringLiteral("^##|v(\\d+)|##"));
	quint32 lineNo = 0;
	bool haveMagic = false;
	PartitionTable* ptable = NULL;
	PartitionTable::TableType tableType = PartitionTable::unknownTableType;

	while (!(line = file.readLine()).isEmpty())
	{
		lineNo++;
		line = line.simplified();

		if (line.isEmpty())
			continue;

		if (!haveMagic && rxMagic.indexIn(QString::fromUtf8(line.constData())) == -1)
		{
			KMessageBox::error(this, xi18nc("@info", "The import file <filename>%1</filename> does not contain a valid partition table.", fileName), i18nc("@title:window", "Error While Importing Partition Table"));
			return;
		}
		else
			haveMagic = true;

		if (line.startsWith('#'))
			continue;

		if (rxType.indexIn(QString::fromUtf8(line.constData())) != -1)
		{
			if (ptable != NULL)
			{
				KMessageBox::error(this, i18nc("@info", "Found more than one partition table type in import file (line %1).", lineNo), i18nc("@title:window", "Error While Importing Partition Table"));
				return;
			}

			tableType = PartitionTable::nameToTableType(rxType.cap(1));

			if (tableType == PartitionTable::unknownTableType)
			{
				KMessageBox::error(this, i18nc("@info", "Partition table type \"%1\" is unknown (line %2).", rxType.cap(1), lineNo), i18nc("@title:window", "Error While Importing Partition Table"));
				return;
			}

			if (tableType != PartitionTable::msdos && tableType != PartitionTable::gpt)
			{
				KMessageBox::error(this, i18nc("@info", "Partition table type \"%1\" is not supported for import (line %2).", rxType.cap(1), lineNo), i18nc("@title:window", "Error While Importing Partition Table"));
				return;
			}

			ptable = new PartitionTable(tableType, PartitionTable::defaultFirstUsable(device, tableType), PartitionTable::defaultLastUsable(device, tableType));
			operationStack().push(new CreatePartitionTableOperation(device, ptable));
		}
		else if (rxAlign.indexIn(QString::fromUtf8(line.constData())) != -1)
		{
			// currently ignored
		}
		else if (rxPartition.indexIn(QString::fromUtf8(line.constData())) != -1)
		{
			if (ptable == NULL)
			{
				KMessageBox::error(this, i18nc("@info", "Found partition but no partition table type (line %1).",  lineNo), i18nc("@title:window", "Error While Importing Partition Table"));
				return;
			}

			qint32 num = rxPartition.cap(1).toInt();
			qint64 firstSector = rxPartition.cap(2).toLongLong();
			qint64 lastSector = rxPartition.cap(3).toLongLong();
			QString fsName = rxPartition.cap(4);
			QString roleNames = rxPartition.cap(5);
			QString volumeLabel = rxPartition.cap(6).replace(QStringLiteral("\""), QString());
			QStringList flags = rxPartition.cap(7).replace(QStringLiteral("\""), QString()).split(QStringLiteral(","));

			if (firstSector < ptable->firstUsable() || lastSector > ptable->lastUsable())
			{
				KMessageBox::error(this, i18nc("@info the partition is NOT a device path, just a number", "Partition %1 would be outside the device's boundaries (line %2).", num, lineNo), i18nc("@title:window", "Error While Importing Partition Table"));
				return;
			}

			if (firstSector >= lastSector)
			{
				KMessageBox::error(this, i18nc("@info", "Partition %1 has end before start sector (line %2).", num, lineNo), i18nc("@title:window", "Error While Importing Partition Table"));
				return;
			}

			PartitionNode* parent = ptable;

			Q_ASSERT(parent);

			PartitionRole role(PartitionRole::None);

			if (roleNames == QStringLiteral("extended"))
				role = PartitionRole(PartitionRole::Extended);
			else if (roleNames == QStringLiteral("logical"))
			{
				role = PartitionRole(PartitionRole::Logical);
				parent = ptable->findPartitionBySector(firstSector, PartitionRole(PartitionRole::Extended));
			}
			else if (roleNames == QStringLiteral("primary"))
				role = PartitionRole(PartitionRole::Primary);

			if (role == PartitionRole(PartitionRole::None))
			{
				KMessageBox::error(this, i18nc("@info the partition is NOT a device path, just a number", "Unrecognized partition role \"%1\" for partition %2 (line %3).", roleNames, num, lineNo), i18nc("@title:window", "Error While Importing Partition Table"));
				return;
			}

			if (parent == NULL)
			{
				KMessageBox::error(this, i18nc("@info the partition is NOT a device path, just a number", "No parent partition or partition table found for partition %1 (line %2).", num, lineNo), i18nc("@title:window", "Error While Importing Partition Table"));
				return;
			}

			if (role.has(PartitionRole::Extended) && !PartitionTable::tableTypeSupportsExtended(tableType))
			{
				KMessageBox::error(this, i18nc("@info", "The partition table type \"%1\" does not support extended partitions, but one was found (line %2).", PartitionTable::tableTypeToName(tableType), lineNo), i18nc("@title:window", "Error While Importing Partition Table"));
				return;
			}

			FileSystem* fs = FileSystemFactory::create(FileSystem::typeForName(fsName), firstSector, lastSector);

			if (fs == NULL)
			{
				KMessageBox::error(this, i18nc("@info the partition is NOT a device path, just a number", "Could not create file system \"%1\" for partition %2 (line %3).", fsName, num, lineNo), i18nc("@title:window", "Error While Importing Partition Table"));
				return;
			}

			if (fs->supportSetLabel() != FileSystem::cmdSupportNone && !volumeLabel.isEmpty())
				fs->setLabel(volumeLabel);

			Partition* p = new Partition(parent, device, role, fs, firstSector, lastSector, QString(), PartitionTable::FlagNone, QString(), false, PartitionTable::FlagNone, Partition::StateNew);

			operationStack().push(new NewOperation(device, p));
		}
		else
			Log(Log::warning) << i18nc("@info/plain", "Could not parse line %1 from import file. Ignoring it.", lineNo);
	}

	if (ptable->type() == PartitionTable::msdos && ptable->isSectorBased(device))
		ptable->setType(device, PartitionTable::msdos_sectorbased);
}

void MainWindow::onExportPartitionTable()
{
	Q_ASSERT(pmWidget().selectedDevice());
	Q_ASSERT(pmWidget().selectedDevice()->partitionTable());

	const QUrl url = QFileDialog::getSaveFileUrl();

	if (url.isEmpty())
		return;

	QTemporaryFile tempFile;

	if (!tempFile.open())
	{
		KMessageBox::error(this, xi18nc("@info", "Could not create temporary file when trying to save to <filename>%1</filename>.", url.fileName()), i18nc("@title:window", "Error Exporting Partition Table"));
		return;
	}

	QTextStream stream(&tempFile);

	stream << "##|v1|## partition table of " << pmWidget().selectedDevice()->deviceNode() << "\n";
	stream << "# on " << QDateTime::currentDateTime().toString() << "\n";
	stream << *pmWidget().selectedDevice()->partitionTable();

	tempFile.close();

	KIO::CopyJob* job = KIO::move(QUrl::fromLocalFile(tempFile.fileName()), url, KIO::HideProgressInfo);
	job->exec();
	if ( job->error() )
		job->ui()->showErrorMessage();
}

void MainWindow::onFileSystemSupport()
{
	FileSystemSupportDialog dlg(this);
	dlg.exec();
}

void MainWindow::onSettingsChanged()
{
// 	FIXME: port KF5
	if (CoreBackendManager::self()->backend()->about().productName() != Config::backend())
	{
		CoreBackendManager::self()->unload();
		// FIXME: if loadBackend() fails to load the configured backend and loads the default
		// one instead it also sets the default backend in the config; the config dialog will
		// overwrite that again, however, after we're done here.
		if (loadBackend())
		{
			deviceScanner().setupConnections();
			scanDevices();
			FileSystemFactory::init();
		}
		else
			close();
	}

	enableActions();
	pmWidget().updatePartitions();
}

void MainWindow::onConfigureOptions()
{
	if (ConfigureOptionsDialog::showDialog(QStringLiteral("Settings")))
		return;

	QPointer<ConfigureOptionsDialog> dlg = new ConfigureOptionsDialog(this, operationStack(), QStringLiteral("Settings"));

	// FIXME: we'd normally use settingsChanged(), according to the kde api docs. however, this
	// is emitted each time the user changes any of our own settings (backend, default file system), without
	// applying or clicking ok. so the below is the workaround for that.
	// FIXME: port KF5
	connect(dlg, SIGNAL(applyClicked()), SLOT(onSettingsChanged()));
	connect(dlg, SIGNAL(okClicked()), SLOT(onSettingsChanged()));

	dlg->show();
}

void MainWindow::onSmartStatusDevice()
{
	Q_ASSERT(pmWidget().selectedDevice());

	if (pmWidget().selectedDevice())
	{
		QPointer<SmartDialog> dlg = new SmartDialog(this, *pmWidget().selectedDevice());
		dlg->exec();
		delete dlg;
	}
}

void MainWindow::onPropertiesDevice(const QString&)
{
	Q_ASSERT(pmWidget().selectedDevice());

	if (pmWidget().selectedDevice())
	{
		Device& d = *pmWidget().selectedDevice();

		QPointer<DevicePropsDialog> dlg = new DevicePropsDialog(this, d);
		if (dlg->exec() == QDialog::Accepted)
		{
			if (d.partitionTable()->type() == PartitionTable::msdos && dlg->sectorBasedAlignment())
				d.partitionTable()->setType(d, PartitionTable::msdos_sectorbased);
			else if (d.partitionTable()->type() == PartitionTable::msdos_sectorbased && dlg->cylinderBasedAlignment())
				d.partitionTable()->setType(d, PartitionTable::msdos);

			on_m_OperationStack_devicesChanged();
			pmWidget().updatePartitions();
		}

		delete dlg;
	}
}

static QStringList checkSupportInNode(const PartitionNode* parent)
{
	if (parent == NULL)
		return QStringList();

	QStringList rval;

	foreach(const PartitionNode* node, parent->children())
	{
		const Partition* p = dynamic_cast<const Partition*>(node);

		if (p == NULL)
			continue;

		if (node->children().size() > 0)
			rval << checkSupportInNode(node);

		if (!p->fileSystem().supportToolFound() && !p->fileSystem().supportToolName().name.isEmpty())
			rval << QStringLiteral("<tr>"
					"<td>%1</td>"
					"<td>%2</td>"
					"<td>%3</td>"
					"<td><a href=\"%4\">%4</a></td>"
					"</tr>")
					.arg(p->deviceNode())
					.arg(p->fileSystem().name())
					.arg(p->fileSystem().supportToolName().name)
					.arg(p->fileSystem().supportToolName().url.toString());
	}

	return rval;
}

void MainWindow::checkFileSystemSupport()
{
	QStringList supportList;

	foreach(const Device* d, operationStack().previewDevices())
		supportList << checkSupportInNode(d->partitionTable());

	qSort(supportList.begin(), supportList.end(), naturalLessThan);

	if (!supportList.isEmpty())
		KMessageBox::information(this,
				xi18nc("@info",
					"<para>No support tools were found for file systems currently present on hard disks in this computer:</para>"
					"<table style='margin-top:12px'>"
					"<tr>"
					"<td style='font-weight:bold;padding-right:12px;white-space:nowrap;'>Partition</td>"
					"<td style='font-weight:bold;padding-right:12px;white-space:nowrap;'>File System</td>"
					"<td style='font-weight:bold;padding-right:12px;white-space:nowrap;'>Support Tools</td>"
					"<td style='font-weight:bold;padding-right:12px;white-space:nowrap;'>URL</td>"
					"</tr>"
					"%1"
					"</table>"
					"<para>As long as the support tools for these file systems are not installed you will not be able to modify them.</para>"
					"<para>You should find packages with these support tools in your distribution's package manager.</para>",
				supportList.join(QStringLiteral("\n"))),
				i18nc("@title:window", "Missing File System Support Packages"),
				QStringLiteral("showInformationOnMissingFileSystemSupport"), KMessageBox::Notify | KMessageBox::AllowLink);
}
