/***************************************************************************
 *   Copyright (C) 2009,2010 by Volker Lanz <vl@fidra.de>                  *
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

#if !defined(EDITMOUNTOPTIONSDIALOG__H)

#define EDITMOUNTOPTIONSDIALOG__H

#include <QDialog>

class EditMountOptionsDialogWidget;
class QStringList;
class QString;
class QWidget;

class EditMountOptionsDialog : public QDialog
{
	Q_OBJECT

	public:
		EditMountOptionsDialog(QWidget* parent, const QStringList& options);
		~EditMountOptionsDialog();

	public:
		QStringList options();

	protected:
		EditMountOptionsDialogWidget& widget() { return *m_DialogWidget; }

	private:
		EditMountOptionsDialogWidget* m_DialogWidget;
};

#endif

