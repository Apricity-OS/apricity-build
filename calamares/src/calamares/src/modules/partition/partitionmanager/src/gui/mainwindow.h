/***************************************************************************
 *   Copyright (C) 2008,2009,2010 by Volker Lanz <vl@fidra.de>             *
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

#if !defined(MAINWINDOW__H)

#define MAINWINDOW__H

#include "core/operationrunner.h"
#include "core/operationstack.h"
#include "core/devicescanner.h"

#include "util/libpartitionmanagerexport.h"

#include "ui_mainwindowbase.h"

#include <KXmlGui/KXmlGuiWindow>

class ApplyProgressDialog;
class ScanProgressDialog;
class Device;
class Partition;
class InfoPane;

class KActionCollection;

class QWidget;
class QLabel;
class QCloseEvent;
class QEvent;

/** The application's main window.

	@author Volker Lanz <vl@fidra.de>
*/
class LIBPARTITIONMANAGERPRIVATE_EXPORT MainWindow : public KXmlGuiWindow, public Ui::MainWindowBase
{
	Q_OBJECT
	Q_DISABLE_COPY(MainWindow)

	public:
		explicit MainWindow(QWidget* parent = NULL);

	protected:
		void init();
		void setupObjectNames();
		void setupActions();
		void setupConnections();
		void setupStatusBar();
		void loadConfig();
		void saveConfig() const;
		void updateWindowTitle();
		void updateSeletedDeviceMenu();
		void checkFileSystemSupport();

		void enableActions();

		void closeEvent(QCloseEvent*);
		void changeEvent(QEvent* event);

		void setSavedSelectedDeviceNode(const QString& s) { m_SavedSelectedDeviceNode = s; }
		const QString& savedSelectedDeviceNode() const { return m_SavedSelectedDeviceNode; }

		InfoPane& infoPane() { Q_ASSERT(m_InfoPane); return *m_InfoPane; }

		PartitionManagerWidget& pmWidget() { Q_ASSERT(m_PartitionManagerWidget); return *m_PartitionManagerWidget; }
		const PartitionManagerWidget& pmWidget() const { Q_ASSERT(m_PartitionManagerWidget); return *m_PartitionManagerWidget; }

		ListDevices& listDevices() { Q_ASSERT(m_ListDevices); return *m_ListDevices; }
		const ListDevices& listDevices() const { Q_ASSERT(m_ListDevices); return *m_ListDevices; }

		ListOperations& listOperations() { Q_ASSERT(m_ListOperations); return *m_ListOperations; }
		const ListOperations& listOperations() const { Q_ASSERT(m_ListOperations); return *m_ListOperations; }

		TreeLog& treeLog() { Q_ASSERT(m_TreeLog); return *m_TreeLog; }
		const TreeLog& treeLog() const { Q_ASSERT(m_TreeLog); return *m_TreeLog; }

		QDockWidget& dockInformation() { Q_ASSERT(m_DockInformation); return *m_DockInformation; }
		const QDockWidget& dockInformation() const { Q_ASSERT(m_DockInformation); return *m_DockInformation; }

		QDockWidget& dockDevices() { Q_ASSERT(m_DockDevices); return *m_DockDevices; }
		const QDockWidget& dockDevices() const { Q_ASSERT(m_DockDevices); return *m_DockDevices; }

		QDockWidget& dockOperations() { Q_ASSERT(m_DockOperations); return *m_DockOperations; }
		const QDockWidget& dockOperations() const { Q_ASSERT(m_DockOperations); return *m_DockOperations; }

		QDockWidget& dockLog() { Q_ASSERT(m_DockLog); return *m_DockLog; }
		const QDockWidget& dockLog() const { Q_ASSERT(m_DockLog); return *m_DockLog; }

		QLabel& statusText() { Q_ASSERT(m_StatusText); return *m_StatusText; }
		const QLabel& statusText() const { Q_ASSERT(m_StatusText); return *m_StatusText; }

		OperationStack& operationStack() { Q_ASSERT(m_OperationStack); return *m_OperationStack; }
		const OperationStack& operationStack() const { Q_ASSERT(m_OperationStack); return *m_OperationStack; }

		OperationRunner& operationRunner() { Q_ASSERT(m_OperationRunner); return *m_OperationRunner; }
		const OperationRunner& operationRunner() const { Q_ASSERT(m_OperationRunner); return *m_OperationRunner; }

		DeviceScanner& deviceScanner() { Q_ASSERT(m_DeviceScanner); return *m_DeviceScanner; }
		const DeviceScanner& deviceScanner() const { Q_ASSERT(m_DeviceScanner); return *m_DeviceScanner; }

		ApplyProgressDialog& applyProgressDialog() { Q_ASSERT(m_ApplyProgressDialog); return *m_ApplyProgressDialog; }
		const ApplyProgressDialog& applyProgressDialog() const { Q_ASSERT(m_ApplyProgressDialog); return *m_ApplyProgressDialog; }

		ScanProgressDialog& scanProgressDialog() { Q_ASSERT(m_ScanProgressDialog); return *m_ScanProgressDialog; }
		const ScanProgressDialog& scanProgressDialog() const { Q_ASSERT(m_ScanProgressDialog); return *m_ScanProgressDialog; }

	protected Q_SLOTS:
		void on_m_PartitionManagerWidget_selectedPartitionChanged(const Partition* p);
		void on_m_PartitionManagerWidget_contextMenuRequested(const QPoint& pos);
		void on_m_PartitionManagerWidget_deviceDoubleClicked(const Device*);
		void on_m_PartitionManagerWidget_partitionDoubleClicked(const Partition*);

		void on_m_DockInformation_dockLocationChanged(Qt::DockWidgetArea);

		void on_m_OperationStack_operationsChanged();
		void on_m_OperationStack_devicesChanged();

		void on_m_DeviceScanner_finished();
		void on_m_DeviceScanner_progress(const QString& device_node, int percent);

		void on_m_ApplyProgressDialog_finished();

		void on_m_ListDevices_contextMenuRequested(const QPoint& pos);
		void on_m_ListDevices_selectionChanged(const QString& device_node);

		void on_m_TreeLog_contextMenuRequested(const QPoint& pos);
		void on_m_ListOperations_contextMenuRequested(const QPoint& pos);

		void scanDevices();

		void onRefreshDevices();
		void onCreateNewPartitionTable();
		void onExportPartitionTable();
		void onImportPartitionTable();

		void onApplyAllOperations();
		void onUndoOperation();
		void onClearAllOperations();

		void onConfigureOptions();
		void onSettingsChanged();

		void onFileSystemSupport();

		void onSmartStatusDevice();
		void onPropertiesDevice(const QString& device_node = QString());
		void onSelectedDeviceMenuTriggered(bool);

	private:
		OperationStack* m_OperationStack;
		OperationRunner* m_OperationRunner;
		DeviceScanner* m_DeviceScanner;
		ApplyProgressDialog* m_ApplyProgressDialog;
		ScanProgressDialog* m_ScanProgressDialog;
		QLabel* m_StatusText;
		QString m_SavedSelectedDeviceNode;
};

#endif
