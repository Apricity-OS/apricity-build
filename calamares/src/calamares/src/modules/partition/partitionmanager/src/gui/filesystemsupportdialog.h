/***************************************************************************
 *   Copyright (C) 2008, 2010 by Volker Lanz <vl@fidra.de>                 *
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

#if !defined(FILESYSTEMSUPPORTDIALOG__H)

#define FILESYSTEMSUPPORTDIALOG__H

#include <QWidget>
#include <QDialog>

class QDialogButtonBox;
class QPushButton;

class FileSystemSupportDialogWidget;

/** Show supported Operations

	Dialog to show which Operations are supported for which type of FileSystem.

	@author Volker Lanz <vl@fidra.de>
*/
class FileSystemSupportDialog : public QDialog
{
	Q_OBJECT
	Q_DISABLE_COPY(FileSystemSupportDialog)

	public:
		FileSystemSupportDialog(QWidget* parent);
		~FileSystemSupportDialog();

	public:
		QSize sizeHint() const;

	protected Q_SLOTS:
		void onButtonRescanClicked();

	protected:
		FileSystemSupportDialogWidget& dialogWidget() { Q_ASSERT(m_FileSystemSupportDialogWidget); return *m_FileSystemSupportDialogWidget; }
		const FileSystemSupportDialogWidget& dialogWidget() const { Q_ASSERT(m_FileSystemSupportDialogWidget); return *m_FileSystemSupportDialogWidget; }
		void setupDialog();
		void setupConnections();

	private:
		FileSystemSupportDialogWidget* m_FileSystemSupportDialogWidget;
		QDialogButtonBox* dialogButtonBox;
};

#endif
