/***************************************************************************
 *   Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                       *
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

#if !defined(SMARTSTATUS__H)

#define SMARTSTATUS__H

#include <qglobal.h>
#include <QString>
#include <QList>

class SmartAttribute;

struct SkSmartAttributeParsedData;
struct SkDisk;

class SmartStatus
{
	public:
		enum Overall
		{
			Good,
			BadPast,
			BadSectors,
			BadNow,
			BadSectorsMany,
			Bad
		};

		enum SelfTestStatus
		{
			Success,
			Aborted,
			Interrupted,
			Fatal,
			ErrorUnknown,
			ErrorEletrical,
			ErrorServo,
			ErrorRead,
			ErrorHandling,
			InProgress
		};

	public:
		typedef QList<SmartAttribute> Attributes;

	public:
		SmartStatus(const QString& device_path);

	public:
		void update();

		const QString& devicePath() const { return m_DevicePath; }
		bool isValid() const { return m_InitSuccess; }
		bool status() const { return m_Status; }
		const QString& modelName() const { return m_ModelName; }
		const QString& serial() const { return m_Serial; }
		const QString& firmware() const { return m_Firmware; }
		qint64 temp() const { return m_Temp; }
		qint64 badSectors() const { return m_BadSectors; }
		qint64 powerCycles() const { return m_PowerCycles; }
		qint64 poweredOn() const { return m_PoweredOn; }
		const Attributes& attributes() const { return m_Attributes; }
		Overall overall() const { return m_Overall; }
		SelfTestStatus selfTestStatus() const { return m_SelfTestStatus; }

		static QString tempToString(qint64 mkelvin);
		static QString overallAssessmentToString(Overall o);
		static QString selfTestStatusToString(SmartStatus::SelfTestStatus s);

	protected:
		void setStatus(bool s) { m_Status = s; }
		void setModelName(const QString& name) { m_ModelName = name; }
		void setSerial(const QString& s) { m_Serial = s; }
		void setFirmware(const QString& f) { m_Firmware = f; }
		void setTemp(qint64 t) { m_Temp = t; }
		void setInitSuccess(bool b) { m_InitSuccess = b; }
		void setBadSectors(qint64 s) { m_BadSectors = s; }
		void setPowerCycles(qint64 p) { m_PowerCycles = p; }
		void setPoweredOn(qint64 t) { m_PoweredOn = t; }
		void setOverall(Overall o) { m_Overall = o; }
		void setSelfTestStatus(SelfTestStatus s) { m_SelfTestStatus = s; }

		static void callback(SkDisk* skDisk, const SkSmartAttributeParsedData* a, void* user_data);

	private:
		const QString m_DevicePath;
		bool m_InitSuccess;
		bool m_Status;
		QString m_ModelName;
		QString m_Serial;
		QString m_Firmware;
		Overall m_Overall;
		SelfTestStatus m_SelfTestStatus;
		qint64 m_Temp;
		qint64 m_BadSectors;
		qint64 m_PowerCycles;
		qint64 m_PoweredOn;
		Attributes m_Attributes;
};

#endif
