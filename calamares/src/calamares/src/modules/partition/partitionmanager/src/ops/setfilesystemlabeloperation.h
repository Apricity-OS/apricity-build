/***************************************************************************
 *   Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                       *
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

#if !defined(SETFILESYSTEMLABELOPERATION__H)

#define SETFILESYSTEMLABELOPERATION__H

#include "ops/operation.h"

#include <QString>

class OperationStack;
class Partition;

class SetFileSystemLabelJob;

/** Set a FileSystem label.

	Sets the FileSystem label for the given Partition.

	@author Volker Lanz <vl@fidra.de>
*/
class SetFileSystemLabelOperation : public Operation
{
	friend class OperationStack;

	Q_OBJECT
	Q_DISABLE_COPY(SetFileSystemLabelOperation)

	public:
		SetFileSystemLabelOperation(Partition& p, const QString& newlabel);

	public:
		QString iconName() const { return QStringLiteral("edit-rename"); }
		QString description() const;
		void preview();
		void undo();

		virtual bool targets(const Device& d) const;
		virtual bool targets(const Partition& p) const;

	protected:
		Partition& labeledPartition() { return m_LabeledPartition; }
		const Partition& labeledPartition() const { return m_LabeledPartition; }

		const QString& oldLabel() const { return m_OldLabel; }
		const QString& newLabel() const { return m_NewLabel; }

		void setOldLabel(const QString& l) { m_OldLabel = l; }

		SetFileSystemLabelJob* labelJob() { return m_LabelJob; }

	private:
		Partition& m_LabeledPartition;
		QString m_OldLabel;
		QString m_NewLabel;
		SetFileSystemLabelJob* m_LabelJob;
};

#endif
