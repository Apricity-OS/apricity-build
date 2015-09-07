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

#if !defined(PARTITIONMANAGERWIDGET__H)

#define PARTITIONMANAGERWIDGET__H

#include "core/operationstack.h"
#include "core/operationrunner.h"

#include "util/libpartitionmanagerexport.h"

#include "ui_partitionmanagerwidgetbase.h"

#include <QWidget>

class Partition;
class PartWidget;
class Device;

class QWidget;
class QLabel;
class QPoint;

/** The central widget for the application.

	@author Volker Lanz <vl@fidra.de>
*/
class LIBPARTITIONMANAGERPRIVATE_EXPORT PartitionManagerWidget : public QWidget, Ui::PartitionManagerWidgetBase
{
	Q_OBJECT
	Q_DISABLE_COPY(PartitionManagerWidget)

	public:
		PartitionManagerWidget(QWidget* parent = NULL);
		virtual ~PartitionManagerWidget();

	Q_SIGNALS:
		void selectedPartitionChanged(const Partition*);
		void contextMenuRequested(const QPoint&);
		void deviceDoubleClicked(const Device*);
		void partitionDoubleClicked(const Partition* p);

	public Q_SLOTS:
		void setSelectedDevice(Device* d);
		void setSelectedDevice(const QString& device_node);

		void onNewPartition();
		void onResizePartition();
		void onDeletePartition(bool shred = false);
		void onShredPartition();

		void onCopyPartition();
		void onPastePartition();

		void onEditMountPoint();
		void onMountPartition();

		void onCheckPartition();

		void onBackupPartition();
		void onRestorePartition();

		void onPropertiesPartition();

	public:
		void init(OperationStack* ostack);

		void clear();

		Device* selectedDevice() { return m_SelectedDevice; }
		const Device* selectedDevice() const { return m_SelectedDevice; }

		Partition* selectedPartition();
		void setSelectedPartition(const Partition* p);

		Partition* clipboardPartition() { return m_ClipboardPartition; }
		const Partition* clipboardPartition() const { return m_ClipboardPartition; }
		void setClipboardPartition(Partition* p) { m_ClipboardPartition = p; }

		void updatePartitions();

	protected:
		OperationStack& operationStack() { return *m_OperationStack; }
		const OperationStack& operationStack() const { return *m_OperationStack; }

		void setupConnections();
		void loadConfig();
		void saveConfig() const;
		bool showInsertDialog(Partition& insertPartition, qint64 sourceLength);

		PartTableWidget& partTableWidget() { Q_ASSERT(m_PartTableWidget); return *m_PartTableWidget; }
		const PartTableWidget& partTableWidget() const { Q_ASSERT(m_PartTableWidget); return *m_PartTableWidget; }

		QTreeWidget& treePartitions() { Q_ASSERT(m_TreePartitions); return *m_TreePartitions; }
		const QTreeWidget& treePartitions() const { Q_ASSERT(m_TreePartitions); return *m_TreePartitions; }

	protected Q_SLOTS:
		void on_m_TreePartitions_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
		void on_m_TreePartitions_customContextMenuRequested(const QPoint& pos);
		void on_m_TreePartitions_itemDoubleClicked(QTreeWidgetItem* item, int);

		void on_m_PartTableWidget_itemSelectionChanged(PartWidget* item);
		void on_m_PartTableWidget_customContextMenuRequested(const QPoint& pos);
		void on_m_PartTableWidget_itemDoubleClicked();

		void onHeaderContextMenu(const QPoint& p);

	private:
		OperationStack* m_OperationStack;
		Device* m_SelectedDevice;
		Partition* m_ClipboardPartition;
};

#endif

