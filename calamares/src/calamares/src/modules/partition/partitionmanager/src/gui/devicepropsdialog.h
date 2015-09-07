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

#if !defined(DEVICEPROPSDIALOG__H)

#define DEVICEPROPSDIALOG__H

#include <QDialog>

class Device;
class DevicePropsWidget;

class QDialogButtonBox;
class QPushButton;
class QVBoxLayout;
class QWidget;
class QString;

/** Show Device properties.

	Dialog that shows a Device's properties.

	@author Volker Lanz <vl@fidra.de>
*/
class DevicePropsDialog : public QDialog
{
	Q_OBJECT
	Q_DISABLE_COPY(DevicePropsDialog)

	public:
		DevicePropsDialog(QWidget* parent, Device& d);
		~DevicePropsDialog();

	public:
		bool cylinderBasedAlignment() const;
		bool sectorBasedAlignment() const;

	protected:
		void setupDialog();
		void setupConnections();

		Device& device() { return m_Device; }
		const Device& device() const { return m_Device; }

		DevicePropsWidget& dialogWidget() { Q_ASSERT(m_DialogWidget); return *m_DialogWidget; }
		const DevicePropsWidget& dialogWidget() const { Q_ASSERT(m_DialogWidget); return *m_DialogWidget; }

		void onButtonSmartMore();

	protected Q_SLOTS:
		void setDirty(bool);
		void onButtonSmartMore(bool);

	private:
		Device& m_Device;
		DevicePropsWidget* m_DialogWidget;

		QDialogButtonBox* dialogButtonBox;
		QPushButton* okButton;
		QPushButton* cancelButton;
		QVBoxLayout *mainLayout;
};

#endif
