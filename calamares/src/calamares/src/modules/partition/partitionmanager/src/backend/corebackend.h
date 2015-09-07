/***************************************************************************
 *   Copyright (C) 2010 by Volker Lanz <vl@fidra.de                        *
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

#if !defined(COREBACKEND__H)

#define COREBACKEND__H

#include "util/libpartitionmanagerexport.h"

#include <QObject>
#include <QList>

class CoreBackendManager;
class CoreBackendDevice;
class Device;
class PartitionTable;

class KAboutData;

class QString;

/**
  * Interface class for backend plugins.
  * @author Volker Lanz <vl@fidra.de>
  */

class LIBPARTITIONMANAGERPRIVATE_EXPORT CoreBackend : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(CoreBackend)

	friend class CoreBackendManager;

	protected:
		CoreBackend();
		virtual ~CoreBackend();

	Q_SIGNALS:
		/**
		 * Emitted to inform about progress of any kind.
		  * @param i the progress in percent (from 0 to 100)
		 */
		void progress(int i);

		/**
		  * Emitted to inform about scan progress.
		  * @param device_node the device being scanned just now (e.g. "/dev/sda")
		  * @param i the progress in percent (from 0 to 100)
		  */
		void scanProgress(const QString& device_node, int i);

	public:
		/**
		  * Return the plugin's KAboutData
		  * @return the plugin's KAboutData
		  */
		virtual const KAboutData& about() const { return *m_AboutData; }

		/**
		  * Initialize the plugin's FileSystem support
		  */
		virtual void initFSSupport() = 0;

		/**
		  * Scan for devices in the system.
		  * @return a QList of pointers to Device instances. The caller is responsible
		  *         for deleting these objects.
		  */
		virtual QList<Device*> scanDevices() = 0;

		/**
		  * Scan a single device in the system.
		  * @param device_node The path to the device that is to be scanned (e.g. /dev/sda)
		  * @return a pointer to a Device instance. The caller is responsible for deleting
		  *         this object.
		  */
		virtual Device* scanDevice(const QString& device_node) = 0;

		/**
		  * Open a device for reading.
		  * @param device_node The path of the device that is to be opened (e.g. /dev/sda)
		  * @return a pointer to a CoreBackendDevice or NULL if the open failed. If a pointer to
		  *         an instance is returned, it's the caller's responsibility to delete the
		  *         object.
		  */
		virtual CoreBackendDevice* openDevice(const QString& device_node) = 0;

		/**
		  * Open a device in exclusive mode for writing.
		  * @param device_node The path of the device that is to be opened (e.g. /dev/sda)
		  * @return a pointer to a CoreBackendDevice or NULL if the open failed. If a pointer to
		  *         an instance is returned, it's the caller's responsibility to delete the
		  *         object.
		  */
		virtual CoreBackendDevice* openDeviceExclusive(const QString& device_node) = 0;

		/**
		  * Close a CoreBackendDevice that has previously been opened.
		  * @param core_device Pointer to the CoreBackendDevice to be closed. Must not be NULL.
		  * @return true if closing the CoreBackendDevice succeeded, otherwise false.
		  *
		  * This method does not delete the object.
		  */
		virtual bool closeDevice(CoreBackendDevice* core_device) = 0;

		/**
		  * Emit progress.
		  * @param i the progress in percent (from 0 to 100)
		  * This is used to emit a progress() signal from somewhere deep inside the plugin
		  * backend code if that is ever necessary.
		  */
		virtual void emitProgress(int i);

		/**
		  * Emit scan progress.
		  * @param device_node the path to the device just being scanned (e.g. /dev/sda)
		  * @param i the progress in percent (from 0 to 100)
		  * This is used to emit a scanProgress() signal from the backend device scanning
		  * code.
		  */
		virtual void emitScanProgress(const QString& device_node, int i);

	protected:
		static void setPartitionTableForDevice(Device& d, PartitionTable* p);
		static void setPartitionTableMaxPrimaries(PartitionTable& p, qint32 max_primaries);

	private:
		void setAboutData(const KAboutData* a) { m_AboutData = a; }

	private:
		const KAboutData* m_AboutData;

		class CoreBackendPrivate;
		CoreBackendPrivate* d;
};

#endif
