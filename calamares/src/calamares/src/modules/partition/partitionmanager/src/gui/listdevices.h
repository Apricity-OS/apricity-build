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

#if !defined(LISTDEVICES__H)

#define LISTDEVICES__H

#include "util/libpartitionmanagerexport.h"

#include "core/operationstack.h"

#include "ui_listdevicesbase.h"

#include <QWidget>

class Device;
class QPoint;
class KActionCollection;

/** A list of devices.
	@author Volker Lanz <vl@fidra.de>
*/
class LIBPARTITIONMANAGERPRIVATE_EXPORT ListDevices : public QWidget, public Ui::ListDevicesBase
{
	Q_OBJECT
	Q_DISABLE_COPY(ListDevices)

	public:
		ListDevices(QWidget* parent = NULL);

	Q_SIGNALS:
		void selectionChanged(const QString& device_node);
		void deviceDoubleClicked(const QString& device_node);
		void contextMenuRequested(const QPoint&);

	public:
		void setActionCollection(KActionCollection* coll) { m_ActionCollection = coll; }
		bool setSelectedDevice(const QString& device_node);

	public Q_SLOTS:
		void updateDevices(OperationStack::Devices& devices);

	protected:
		QListWidget& listDevices() { Q_ASSERT(m_ListDevices); return *m_ListDevices; }
		const QListWidget& listDevices() const { Q_ASSERT(m_ListDevices); return *m_ListDevices; }
		KActionCollection* actionCollection() { return m_ActionCollection; }

	protected Q_SLOTS:
		void on_m_ListDevices_itemSelectionChanged();
		void on_m_ListDevices_customContextMenuRequested(const QPoint& pos);
		void on_m_ListDevices_itemDoubleClicked(QListWidgetItem* list_item);

	private:
		KActionCollection* m_ActionCollection;
};

#endif

