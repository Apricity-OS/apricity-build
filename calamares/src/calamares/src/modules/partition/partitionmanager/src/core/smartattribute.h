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

#if !defined(SMARTATTRIBUTE__H)

#define SMARTATTRIBUTE__H

#include <QString>

struct SkSmartAttributeParsedData;

class SmartAttribute
{
	public:
		enum FailureType
		{
			PreFailure,
			OldAge
		};

		enum UpdateType
		{
			Online,
			Offline
		};

		enum Assessment
		{
			NotApplicable,
			Failing,
			HasFailed,
			Warning,
			Good
		};

	public:
		SmartAttribute(const SkSmartAttributeParsedData* a);

	public:
		qint32 id() const { return m_Id; }
		const QString& name() const { return m_Name; }
		const QString& desc() const { return m_Desc; }
		FailureType failureType() const { return m_FailureType; }
		UpdateType updateType() const { return m_UpdateType; }
		qint32 current() const { return m_Current; }
		qint32 worst() const { return m_Worst; }
		qint32 threshold() const { return m_Threshold; }
		const QString& raw() const { return m_Raw; }
		Assessment assessment() const { return m_Assessment; }
		const QString& value() const { return m_Value; }

		QString assessmentToString() const { return assessmentToString(assessment()); }
		static QString assessmentToString(Assessment a);

	private:
		qint32 m_Id;
		QString m_Name;
		QString m_Desc;
		FailureType m_FailureType;
		UpdateType m_UpdateType;
		qint32 m_Current;
		qint32 m_Worst;
		qint32 m_Threshold;
		QString m_Raw;
		Assessment m_Assessment;
		QString m_Value;
};

#endif

