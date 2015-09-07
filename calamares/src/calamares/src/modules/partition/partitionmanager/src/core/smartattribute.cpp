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

#include "core/smartattribute.h"
#include "core/smartstatus.h"

#include <QLocale>

#include <KLocalizedString>
#include <KFormat>

#include <atasmart.h>

static QString getAttrName(qint32 id);
static QString getAttrDescription(qint32 id);
static QString getPrettyValue(qint64 value, qint64 unit);
static SmartAttribute::Assessment getAssessment(const SkSmartAttributeParsedData* a);
static QString getRaw(const uint8_t*);

SmartAttribute::SmartAttribute(const SkSmartAttributeParsedData* a) :
	m_Id(a->id),
	m_Name(getAttrName(a->id)),
	m_Desc(getAttrDescription(a->id)),
	m_FailureType(a->prefailure ? PreFailure : OldAge),
	m_UpdateType(a->online ? Online : Offline),
	m_Current(a->current_value_valid ?  a->current_value : -1),
	m_Worst(a->worst_value_valid ? a->worst_value : -1),
	m_Threshold(a->threshold_valid ? a->threshold : -1),
	m_Raw(getRaw(a->raw)),
	m_Assessment(getAssessment(a)),
	m_Value(getPrettyValue(a->pretty_value, a->pretty_unit))
{
}

QString SmartAttribute::assessmentToString(Assessment a)
{
	switch(a)
	{
		case Failing:
			return i18nc("@item:intable", "failing");

		case HasFailed:
			return i18nc("@item:intable", "has failed");

		case Warning:
			return i18nc("@item:intable", "warning");

		case Good:
			return i18nc("@item:intable", "good");

		case NotApplicable:
		default:
			return i18nc("@item:intable not applicable", "N/A");
	}
}

static QString getPrettyValue(qint64 value, qint64 unit)
{
	QString rval;

	switch (unit)
	{
		case SK_SMART_ATTRIBUTE_UNIT_MSECONDS:
			rval = KFormat().formatDuration(value);
			break;

		case SK_SMART_ATTRIBUTE_UNIT_SECTORS:
			rval = i18ncp("@item:intable", "%1 sector", "%1 sectors", value);
			break;

		case SK_SMART_ATTRIBUTE_UNIT_MKELVIN:
			rval = SmartStatus::tempToString(value);
			break;

		case SK_SMART_ATTRIBUTE_UNIT_NONE:
			rval = QLocale().toString(value);
			break;

		case SK_SMART_ATTRIBUTE_UNIT_UNKNOWN:
		default:
			rval = i18nc("@item:intable not applicable", "N/A");
			break;
	}

	return rval;
}

typedef struct
{
	qint32 id;
	const QString name;
	const QString desc;
} AttrDetails;

static const AttrDetails* attrDetails()
{
	static const AttrDetails details[] =
	{
		{ 1,  i18nc("SMART attr name", "Read Error Rate"), i18nc("SMART attr description", "Rate of hardware read errors while reading data from the disk surface.")  },
		{ 2,  i18nc("SMART attr name", "Throughput Performance"), i18nc("SMART attr description", "Overall (general) throughput performance of a hard disk drive. If the value of this attribute is decreasing there is a high probability that there is a problem with the disk.")  },
		{ 3,  i18nc("SMART attr name", "Spin-Up Time"), i18nc("SMART attr description", "Average time of spindle spin up from zero RPM to fully operational.")  },
		{ 4,  i18nc("SMART attr name", "Start/Stop Count"), i18nc("SMART attr description", "A tally of spindle start/stop cycles.")  },
		{ 5,  i18nc("SMART attr name", "Reallocated Sectors Count"), i18nc("SMART attr description", "Count of reallocated sectors. When the hard drive finds a read/write/verification error, it marks this sector as &quot;reallocated&quot; and transfers data to a special reserved area (spare area).")  },
		{ 6,  i18nc("SMART attr name", "Read Channel Margin"), i18nc("SMART attr description", "Margin of a channel while reading data. The function of this attribute is not specified.")  },
		{ 7,  i18nc("SMART attr name", "Seek Error Rate"), i18nc("SMART attr description", "Rate of seek errors of the magnetic heads. If there is a partial failure in the mechanical positioning system, then seek errors will arise.")  },
		{ 8,  i18nc("SMART attr name", "Seek Time Performance"), i18nc("SMART attr description", "Average performance of seek operations of the magnetic heads. If this attribute is decreasing, it is a sign of problems in the mechanical subsystem.")  },
		{ 9,  i18nc("SMART attr name", "Power-On Hours"), i18nc("SMART attr description", "Count of hours in power-on state.")  },
		{ 10,  i18nc("SMART attr name", "Spin Retry Count"), i18nc("SMART attr description", "Count of retry of spin start attempts if the first attempt was unsuccessful. An increase of this attribute value is a sign of problems in the hard disk mechanical subsystem.")  },
		{ 11,  i18nc("SMART attr name", "Recalibration Retries"), i18nc("SMART attr description", "Count of recalibrations requested if the first attempt was unsuccessful. An increase of this attribute value is a sign of problems in the hard disk mechanical subsystem.")  },
		{ 12,  i18nc("SMART attr name", "Power Cycle Count"), i18nc("SMART attr description", "Count of full hard disk power on/off cycles.")  },
		{ 13,  i18nc("SMART attr name", "Soft Read Error Rate"), i18nc("SMART attr description", "Uncorrected read errors reported to the operating system.")  },
		{ 183,  i18nc("SMART attr name", "SATA Downshift Error Count"), i18nc("SMART attr description", "Western Digital and Samsung attribute.")  },
		{ 184,  i18nc("SMART attr name", "End-to-End Error"), i18nc("SMART attr description", "Part of HP's SMART IV technology: After transferring through the cache RAM data buffer the parity data between the host and the hard drive did not match.")  },
		{ 185,  i18nc("SMART attr name", "Head Stability"), i18nc("SMART attr description", "Western Digital attribute.")  },
		{ 186,  i18nc("SMART attr name", "Induced Op-Vibration Detection"), i18nc("SMART attr description", "Western Digital attribute.")  },
		{ 187,  i18nc("SMART attr name", "Reported Uncorrectable Errors"), i18nc("SMART attr description", "Count of errors that could not be recovered using hardware ECC.")  },
		{ 188,  i18nc("SMART attr name", "Command Timeout"), i18nc("SMART attr description", "Count of aborted operations due to HDD timeout.")  },
		{ 189,  i18nc("SMART attr name", "High Fly Writes"), i18nc("SMART attr description", "Count of fly height errors detected.")  },
		{ 190,  i18nc("SMART attr name", "Temperature Difference From 100"), i18nc("SMART attr description", "Value is equal to (100 &ndash; temp. °C), allowing manufacturer to set a minimum threshold which corresponds to a maximum temperature.")  },
		{ 191,  i18nc("SMART attr name", "G-sense Error Rate"), i18nc("SMART attr description", "Count of errors resulting from externally-induced shock and vibration.")  },
		{ 192,  i18nc("SMART attr name", "Power Off Retract Count"), i18nc("SMART attr description", "Count of power-off or emergency retract cycles")  },
		{ 193,  i18nc("SMART attr name", "Load Cycle Count"), i18nc("SMART attr description", "Count of load/unload cycles into head landing zone position.")  },
		{ 194,  i18nc("SMART attr name", "Temperature"), i18nc("SMART attr description", "Current internal temperature.")  },
		{ 195,  i18nc("SMART attr name", "Hardware ECC Recovered"), i18nc("SMART attr description", "Count of errors that could be recovered using hardware ECC.")  },
		{ 196,  i18nc("SMART attr name", "Reallocation Event Count"), i18nc("SMART attr description", "Count of remap operations. The raw value of this attribute shows the total number of attempts to transfer data from reallocated sectors to a spare area.")  },
		{ 197,  i18nc("SMART attr name", "Current Pending Sector Count"), i18nc("SMART attr description", "Number of &quot;unstable&quot; sectors (waiting to be remapped, because of read errors).")  },
		{ 198,  i18nc("SMART attr name", "Uncorrectable Sector Count"), i18nc("SMART attr description", "Count of uncorrectable errors when reading/writing a sector.")  },
		{ 199,  i18nc("SMART attr name", "UltraDMA CRC Error Count"), i18nc("SMART attr description", "Count of errors in data transfer via the interface cable as determined by ICRC.")  },
		{ 200,  i18nc("SMART attr name", "Multi-Zone Error Rate<br/>Write Error Rate"), i18nc("SMART attr description", "The total number of errors when writing a sector.")  },
		{ 201,  i18nc("SMART attr name", "Soft Read Error Rate"), i18nc("SMART attr description", "Number of off-track errors.")  },
		{ 202,  i18nc("SMART attr name", "Data Address Mark Errors"), i18nc("SMART attr description", "Number of Data Address Mark errors (or vendor-specific).")  },
		{ 203,  i18nc("SMART attr name", "Run Out Cancel"), i18nc("SMART attr description", "Number of ECC errors")  },
		{ 204,  i18nc("SMART attr name", "Soft ECC Correction"), i18nc("SMART attr description", "Number of errors corrected by software ECC")  },
		{ 205,  i18nc("SMART attr name", "Thermal Asperity Rate"), i18nc("SMART attr description", "Number of errors due to high temperature.")  },
		{ 206,  i18nc("SMART attr name", "Flying Height"), i18nc("SMART attr description", "Height of heads above the disk surface. A flying height that is too low increases the chances of a head crash while a flying height that is too high increases the chances of a read/write error.")  },
		{ 207,  i18nc("SMART attr name", "Spin High Current"), i18nc("SMART attr description", "Amount of surge current used to spin up the drive.")  },
		{ 208,  i18nc("SMART attr name", "Spin Buzz"), i18nc("SMART attr description", "Number of buzz routines needed to spin up the drive due to insufficient power.")  },
		{ 209,  i18nc("SMART attr name", "Offline Seek Performance"), i18nc("SMART attr description", "Drive's seek performance during its internal tests.")  },
		{ 211,  i18nc("SMART attr name", "Vibration During Write"), i18nc("SMART attr description", "Vibration During Write")  },
		{ 212,  i18nc("SMART attr name", "Shock During Write"), i18nc("SMART attr description", "Shock During Write")  },
		{ 220,  i18nc("SMART attr name", "Disk Shift"), i18nc("SMART attr description", "Distance the disk has shifted relative to the spindle (usually due to shock or temperature).")  },
		{ 221,  i18nc("SMART attr name", "G-Sense Error Rate"), i18nc("SMART attr description", "The number of errors resulting from externally-induced shock and vibration.")  },
		{ 222,  i18nc("SMART attr name", "Loaded Hours"), i18nc("SMART attr description", "Time spent operating under data load.")  },
		{ 223,  i18nc("SMART attr name", "Load/Unload Retry Count"), i18nc("SMART attr description", "Number of times head changes position.")  },
		{ 224,  i18nc("SMART attr name", "Load Friction"), i18nc("SMART attr description", "Resistance caused by friction in mechanical parts while operating.")  },
		{ 225,  i18nc("SMART attr name", "Load/Unload Cycle Count"), i18nc("SMART attr description", "Total number of load cycles.")  },
		{ 226,  i18nc("SMART attr name", "Load-In Time"), i18nc("SMART attr description", "Total time of loading on the magnetic heads actuator (time not spent in parking area).")  },
		{ 227,  i18nc("SMART attr name", "Torque Amplification Count"), i18nc("SMART attr description", "Number of attempts to compensate for platter speed variations.")  },
		{ 228,  i18nc("SMART attr name", "Power-Off Retract Cycle"), i18nc("SMART attr description", "The number of times the magnetic armature was retracted automatically as a result of cutting power.")  },
		{ 230,  i18nc("SMART attr name", "GMR Head Amplitude"), i18nc("SMART attr description", "Amplitude of &quot;thrashing&quot; (distance of repetitive forward/reverse head motion)")  },
		{ 231,  i18nc("SMART attr name", "Temperature"), i18nc("SMART attr description", "Drive Temperature")  },
		{ 232,  i18nc("SMART attr name", "Endurance Remaining"), i18nc("SMART attr description", "Count of physical erase cycles completed on the drive as a percentage of the maximum physical erase cycles the drive supports")  },
		{ 233,  i18nc("SMART attr name", "Power-On Seconds"), i18nc("SMART attr description", "Time elapsed in the power-on state")  },
		{ 234,  i18nc("SMART attr name", "Unrecoverable ECC Count"), i18nc("SMART attr description", "Count of unrecoverable ECC errors")  },
		{ 235,  i18nc("SMART attr name", "Good Block Rate"), i18nc("SMART attr description", "Count of available reserved blocks as percentage of the total number of reserved blocks")  },
		{ 240,  i18nc("SMART attr name", "Head Flying Hours<br/>or Transfer Error Rate (Fujitsu)"), i18nc("SMART attr description", "Time while head is positioning<br/>or counts the number of times the link is reset during a data transfer.")  },
		{ 241,  i18nc("SMART attr name", "Total LBAs Written"), i18nc("SMART attr description", "Total LBAs Written")  },
		{ 242,  i18nc("SMART attr name", "Total LBAs Read"), i18nc("SMART attr description", "Total LBAs Read")  },
		{ 250,  i18nc("SMART attr name", "Read Error Retry Rate"), i18nc("SMART attr description", "Number of errors while reading from a disk")  },
		{ 254,  i18nc("SMART attr name", "Free Fall Protection"), i18nc("SMART attr description", "Number of &quot;Free Fall Events&quot; detected") },
		{ -1, QString(), QString() }
	};

	return details;
}

static QString getAttrName(qint32 id)
{
	qint32 idx = 0;

	while (attrDetails()[idx].id != -1)
	{
		if (attrDetails()[idx].id == id)
			return attrDetails()[idx].name;
		idx++;
	}

	return QString();
}

static QString getAttrDescription(qint32 id)
{
	qint32 idx = 0;

	while (attrDetails()[idx].id != -1)
	{
		if (attrDetails()[idx].id == id)
			return attrDetails()[idx].desc;
		idx++;
	}

	return QString();
}

static SmartAttribute::Assessment getAssessment(const SkSmartAttributeParsedData* a)
{
	SmartAttribute::Assessment rval = SmartAttribute::NotApplicable;

	bool failed = false;
	bool hasFailed = false;

	if (a->prefailure)
	{
		if (a->good_now_valid && !a->good_now)
			failed = true;

		if (a->good_in_the_past_valid && !a->good_in_the_past)
			hasFailed = true;
	}
	else if (a->threshold_valid)
	{
		if (a->current_value_valid && a->current_value <= a->threshold)
			failed = true;
		else if (a->worst_value_valid && a->worst_value <= a->threshold)
			hasFailed = true;
	}

	if (failed)
		rval = SmartAttribute::Failing;
	else if (hasFailed)
		rval = SmartAttribute::HasFailed;
	else if (a->warn)
		rval = SmartAttribute::Warning;
	else if (a->good_now_valid)
		rval = SmartAttribute::Good;

	return rval;
}

static QString getRaw(const uint8_t* raw)
{
	QString rval = QStringLiteral("0x");
	for (qint32 i = 5; i >= 0; i--)
		rval += QStringLiteral("%1").arg(raw[i], 2, 16, QLatin1Char('0'));

	return rval;
}
