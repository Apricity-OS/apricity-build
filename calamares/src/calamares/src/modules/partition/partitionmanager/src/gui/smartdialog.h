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

#if !defined(SMARTDIALOG__H)

#define SMARTDIALOG__H

#include <QDialog>

class Device;
class SmartDialogWidget;

class QWidget;
class QString;
class QPoint;
class QDialogButtonBox;

/** Show SMART properties.

	Dialog that shows SMART status and properties for a device

	@author Volker Lanz <vl@fidra.de>
*/
class SmartDialog : public QDialog
{
	Q_OBJECT
	Q_DISABLE_COPY(SmartDialog)

	public:
		SmartDialog(QWidget* parent, Device& d);
		~SmartDialog();

	protected Q_SLOTS:
		void saveSmartReport();

	protected:
		void setupDialog();
		void setupConnections();

		Device& device() { return m_Device; }
		const Device& device() const { return m_Device; }

		SmartDialogWidget& dialogWidget() { Q_ASSERT(m_DialogWidget); return *m_DialogWidget; }
		const SmartDialogWidget& dialogWidget() const { Q_ASSERT(m_DialogWidget); return *m_DialogWidget; }

		QString toHtml() const;

	private:
		Device& m_Device;
		SmartDialogWidget* m_DialogWidget;
		QDialogButtonBox* buttonBox;
};

#endif
