/***************************************************************************
 *   Copyright (C) 2013 by Andrius Štikonas <andrius@stikonas.eu>          *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#if !defined(DECRYPTLUKSDIALOG__H)

#define DECRYPTLUKSDIALOG__H

#include "gui/decryptluksdialogwidget.h"

#include <QDialog>

class Device;

class DecryptLuksDialog : public QDialog
{
	Q_OBJECT

	public:
		DecryptLuksDialog(QWidget* parent, const QString& deviceNode);

	protected:
		DecryptLuksDialogWidget& widget() { return *m_DialogWidget; }
		const DecryptLuksDialogWidget& widget() const { return *m_DialogWidget; }
		const QString& deviceNode() const { return m_DeviceNode; }

	private:
		DecryptLuksDialogWidget* m_DialogWidget;
		const QString& m_DeviceNode;

	public:
		QLineEdit& luksName() { return widget().luksName(); }
		const QLineEdit& luksName() const { return widget().luksName(); }

		QLineEdit& luksPassphrase() { return widget().luksPassphrase(); }
		const QLineEdit& luksPassphrase() const { return widget().luksPassphrase(); }
};


#endif
