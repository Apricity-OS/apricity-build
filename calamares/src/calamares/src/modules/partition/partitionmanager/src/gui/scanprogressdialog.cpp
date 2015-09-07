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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include "gui/scanprogressdialog.h"

#include <KLocalizedString>

ScanProgressDialog::ScanProgressDialog(QWidget* parent) :
	QProgressDialog(parent)
{
	setWindowTitle(i18nc("@title:window", "Scanning devices..."));
	setMinimumWidth(280);
	setMinimumDuration(150);
	setAttribute(Qt::WA_ShowModal, true);
}

void ScanProgressDialog::setDeviceName(const QString& d)
{
	if (d.isEmpty())
		setLabelText(i18nc("@label", "Scanning..."));
	else
		setLabelText(xi18nc("@label", "Scanning device: <filename>%1</filename>", d));
}

void ScanProgressDialog::showEvent(QShowEvent* e)
{
	setCancelButton(0);

	QProgressDialog::showEvent(e);
}
