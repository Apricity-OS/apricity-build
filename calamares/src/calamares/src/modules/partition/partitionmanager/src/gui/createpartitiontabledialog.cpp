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

#include "gui/createpartitiontabledialog.h"
#include "gui/createpartitiontablewidget.h"

#include "core/device.h"
#include "core/partitiontable.h"

#include <QDialogButtonBox>
#include <QPushButton>

#include <KLocalizedString>
#include <KMessageBox>

#include <config.h>

CreatePartitionTableDialog::CreatePartitionTableDialog(QWidget* parent, const Device& d) :
	QDialog(parent),
	m_DialogWidget(new CreatePartitionTableWidget(this)),
	m_Device(d)
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	setLayout(mainLayout);
	mainLayout->addWidget(&widget());
	setWindowTitle(xi18nc("@title:window", "Create a New Partition Table on <filename>%1</filename>", device().deviceNode()));
	dialogButtonBox = new QDialogButtonBox;
	createButton = dialogButtonBox->addButton( QDialogButtonBox::Ok );
	createButton->setText(i18n("Create &New Partition Table"));
	cancelButton = dialogButtonBox->addButton( QDialogButtonBox::Cancel );
	mainLayout->addWidget(dialogButtonBox);

	connect(&widget().radioMSDOS(), SIGNAL(toggled(bool)), SLOT(onMSDOSToggled(bool)));
	connect(dialogButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(dialogButtonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

PartitionTable::TableType CreatePartitionTableDialog::type() const
{
	if (widget().radioGPT().isChecked())
		return PartitionTable::gpt;

	if (widget().radioMSDOS().isChecked() && Config::useCylinderAlignment() == true)
		return PartitionTable::msdos;

	return PartitionTable::msdos_sectorbased;
}

void CreatePartitionTableDialog::onMSDOSToggled(bool on)
{
	if (on && device().totalSectors() > 0xffffffff)
	{
		if (KMessageBox::warningContinueCancel(this,
				xi18nc("@info",
					"<para>Do you really want to create an MS-Dos partition table on <filename>%1</filename>?</para>"
					"<para>This device has more than 2^32 sectors. That is the most the MS-Dos partition table type supports, so you will not be able to use the whole device.</para>", device().deviceNode()),
				i18nc("@title:window", "Really Create MS-Dos Partition Table Type?"),
				KGuiItem(i18nc("@action:button", "Create MS-Dos Type"), QStringLiteral("arrow-right")),
				KStandardGuiItem::cancel(), QStringLiteral("reallyCreateMSDOSOnLargeDevice")) == KMessageBox::Cancel)
		{
			widget().radioGPT().setChecked(true);
		}
	}
}

