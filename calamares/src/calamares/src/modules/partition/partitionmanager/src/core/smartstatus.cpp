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

#include "core/smartstatus.h"
#include "core/smartattribute.h"

#include <KLocalizedString>

#include <QDebug>
#include <QString>
#include <QStringList>

#include <atasmart.h>
#include <errno.h>

SmartStatus::SmartStatus(const QString& device_path) :
	m_DevicePath(device_path),
	m_InitSuccess(false),
	m_Status(false),
	m_ModelName(),
	m_Serial(),
	m_Firmware(),
	m_Overall(Bad),
	m_SelfTestStatus(Success),
	m_Temp(-99),
	m_BadSectors(-99),
	m_PowerCycles(-99),
	m_PoweredOn(-99)
{
	update();
}

void SmartStatus::update()
{
	SkDisk* skDisk = NULL;
	SkBool skSmartStatus = false;
	uint64_t mkelvin = 0;
	uint64_t skBadSectors = 0;
	uint64_t skPoweredOn = 0;
	uint64_t skPowerCycles = 0;

	if (sk_disk_open(devicePath().toLocal8Bit().constData(), &skDisk) < 0)
	{
		qDebug() << "smart disk open failed for " << devicePath() << ": " << strerror(errno);
		return;
	}

	if (sk_disk_smart_status(skDisk, &skSmartStatus) < 0)
	{
		qDebug() << "getting smart status failed for " << devicePath() << ": " << strerror(errno);
		sk_disk_free(skDisk);
		return;
	}

	setStatus(skSmartStatus);

	if (sk_disk_smart_read_data(skDisk) < 0)
	{
		qDebug() << "reading smart data failed for " << devicePath() << ": " << strerror(errno);
		sk_disk_free(skDisk);
		return;
	}

	const SkIdentifyParsedData* skIdentify;

	if (sk_disk_identify_parse(skDisk, &skIdentify) < 0)
		qDebug() << "getting identify data failed for " <<  devicePath() << ": " << strerror(errno);
	else
	{
		setModelName(QString::fromUtf8(skIdentify->model));
		setFirmware(QString::fromUtf8(skIdentify->firmware));
		setSerial(QString::fromUtf8(skIdentify->serial));
	}

	const SkSmartParsedData* skParsed;
	if (sk_disk_smart_parse(skDisk, &skParsed) < 0)
		qDebug() << "parsing disk smart data failed for " <<  devicePath() << ": " << strerror(errno);
	else
	{
		switch(skParsed->self_test_execution_status)
		{
			case SK_SMART_SELF_TEST_EXECUTION_STATUS_ABORTED:
				setSelfTestStatus(Aborted);
				break;

			case SK_SMART_SELF_TEST_EXECUTION_STATUS_INTERRUPTED:
				setSelfTestStatus(Interrupted);
				break;

			case SK_SMART_SELF_TEST_EXECUTION_STATUS_FATAL:
				setSelfTestStatus(Fatal);
				break;

			case SK_SMART_SELF_TEST_EXECUTION_STATUS_ERROR_UNKNOWN:
				setSelfTestStatus(ErrorUnknown);
				break;

			case SK_SMART_SELF_TEST_EXECUTION_STATUS_ERROR_ELECTRICAL:
				setSelfTestStatus(ErrorEletrical);
				break;

			case SK_SMART_SELF_TEST_EXECUTION_STATUS_ERROR_SERVO:
				setSelfTestStatus(ErrorServo);
				break;

			case SK_SMART_SELF_TEST_EXECUTION_STATUS_ERROR_READ:
				setSelfTestStatus(ErrorRead);
				break;

			case SK_SMART_SELF_TEST_EXECUTION_STATUS_ERROR_HANDLING:
				setSelfTestStatus(ErrorHandling);
				break;

			case SK_SMART_SELF_TEST_EXECUTION_STATUS_INPROGRESS:
				setSelfTestStatus(InProgress);
				break;

			default:
			case SK_SMART_SELF_TEST_EXECUTION_STATUS_SUCCESS_OR_NEVER:
				setSelfTestStatus(Success);
				break;
		}
	}

	SkSmartOverall overall;

	if (sk_disk_smart_get_overall(skDisk, &overall) < 0)
		qDebug() << "getting status failed for " <<  devicePath() << ": " << strerror(errno);
	else
	{
		switch(overall)
		{
			case SK_SMART_OVERALL_GOOD:
				setOverall(Good);
				break;

			case SK_SMART_OVERALL_BAD_ATTRIBUTE_IN_THE_PAST:
				setOverall(BadPast);
				break;

			case SK_SMART_OVERALL_BAD_SECTOR:
				setOverall(BadSectors);
				break;

			case SK_SMART_OVERALL_BAD_ATTRIBUTE_NOW:
				setOverall(BadNow);
				break;

			case SK_SMART_OVERALL_BAD_SECTOR_MANY:
				setOverall(BadSectorsMany);
				break;

			default:
			case SK_SMART_OVERALL_BAD_STATUS:
				setOverall(Bad);
				break;
		}
	}

	if (sk_disk_smart_get_temperature(skDisk, &mkelvin) < 0)
		qDebug() << "getting temp failed for " <<  devicePath() << ": " << strerror(errno);
	else
		setTemp(mkelvin);

	if (sk_disk_smart_get_bad(skDisk, &skBadSectors) < 0)
		qDebug() << "getting bad sectors failed for " <<  devicePath() << ": " << strerror(errno);
	else
		setBadSectors(skBadSectors);

	if (sk_disk_smart_get_power_on(skDisk, &skPoweredOn) < 0)
		qDebug() << "getting powered on time failed for " <<  devicePath() << ": " << strerror(errno);
	else
		setPoweredOn(skPoweredOn);

	if (sk_disk_smart_get_power_cycle(skDisk, &skPowerCycles) < 0)
		qDebug() << "getting power cycles failed for " <<  devicePath() << ": " << strerror(errno);
	else
		setPowerCycles(skPowerCycles);

	m_Attributes.clear();

	sk_disk_smart_parse_attributes(skDisk, callback, this);

	sk_disk_free(skDisk);
	setInitSuccess(true);
}

QString SmartStatus::tempToString(qint64 mkelvin)
{
	const double celsius = (mkelvin - 273150.0) / 1000.0;
	const double fahrenheit = 9.0 * celsius / 5.0 + 32;
	return i18nc("@item:intable degrees in Celsius and Fahrenheit", "%1° C / %2° F", QLocale().toString(celsius, 1), QLocale().toString(fahrenheit, 1));
}

QString SmartStatus::selfTestStatusToString(SmartStatus::SelfTestStatus s)
{
	switch(s)
	{
		case Aborted:
			return i18nc("@item", "Aborted");

		case Interrupted:
			return i18nc("@item", "Interrupted");

		case Fatal:
			return i18nc("@item", "Fatal error");

		case ErrorUnknown:
			return i18nc("@item", "Unknown error");

		case ErrorEletrical:
			return i18nc("@item", "Electrical error");

		case ErrorServo:
			return i18nc("@item", "Servo error");

		case ErrorRead:
			return i18nc("@item", "Read error");

		case ErrorHandling:
			return i18nc("@item", "Handling error");

		case InProgress:
			return i18nc("@item", "Self test in progress");

		case Success:
		default:
			return i18nc("@item", "Success");
	}

}

QString SmartStatus::overallAssessmentToString(Overall o)
{
	switch(o)
	{
		case Good:
			return i18nc("@item", "Healthy");

		case BadPast:
			return i18nc("@item", "Has been used outside of its design parameters in the past.");

		case BadSectors:
			return i18nc("@item", "Has some bad sectors.");

		case BadNow:
			return i18nc("@item", "Is being used outside of its design parameters right now.");

		case BadSectorsMany:
			return i18nc("@item", "Has many bad sectors.");

		case Bad:
		default:
			return i18nc("@item", "Disk failure is imminent. Backup all data!");
	}

}

void SmartStatus::callback(SkDisk*, const SkSmartAttributeParsedData* a, void* user_data)
{
	SmartStatus* self = reinterpret_cast<SmartStatus*>(user_data);

	SmartAttribute sm(a);
	self->m_Attributes.append(sm);
}

