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

#include "gui/editmountpointdialogwidget.h"
#include "gui/editmountoptionsdialog.h"

#include "core/partition.h"
#include "core/mountentry.h"

#include "fs/filesystem.h"

#include <KLocalizedString>
#include <KIOCore/KMountPoint>
#include <KMessageBox>

#include <QString>
#include <QWidget>
#include <QFile>
#include <QFileDialog>
#include <QPointer>
#include <QDebug>

#include <mntent.h>
#include <blkid/blkid.h>

static QString findBlkIdDevice(const QString& token, const QString& value)
{
	blkid_cache cache;
	QString rval;

	if (blkid_get_cache(&cache, NULL) == 0)
	{
		if (char* c = blkid_evaluate_tag(token.toLocal8Bit().constData(), value.toLocal8Bit().constData(), &cache))
		{
			rval = QString::fromLocal8Bit(c);
			free(c);
		}

		blkid_put_cache(cache);
	}

	return rval;
}

EditMountPointDialogWidget::EditMountPointDialogWidget(QWidget* parent, const Partition& p) :
	QWidget(parent),
	m_Partition(p)
{
	readMountpoints(QStringLiteral("/etc/fstab"));

	setupUi(this);

	labelName().setText(partition().deviceNode());
	labelType().setText(partition().fileSystem().name());

	if (mountPoints().find(partition().deviceNode()) == mountPoints().end())
		mountPoints()[partition().deviceNode()] = new MountEntry(partition().deviceNode(), QString(), partition().fileSystem().name(), QStringList(), 0, 0, MountEntry::deviceNode);

	MountEntry* entry = mountPoints()[partition().deviceNode()];

	Q_ASSERT(entry);

	if (entry)
	{
		editPath().setText(entry->path);

		spinDumpFreq().setValue(entry->dumpFreq);
		spinPassNumber().setValue(entry->passNumber);

		switch(entry->identifyType)
		{
			case MountEntry::uuid:
				radioUUID().setChecked(true);
				break;

			case MountEntry::label:
				radioLabel().setChecked(true);
				break;

			default:
				radioDeviceNode().setChecked(true);
		}

		boxOptions()[QStringLiteral("ro")] = m_CheckReadOnly;
		boxOptions()[QStringLiteral("users")] = m_CheckUsers;
		boxOptions()[QStringLiteral("noauto")] = m_CheckNoAuto;
		boxOptions()[QStringLiteral("noatime")] = m_CheckNoAtime;
		boxOptions()[QStringLiteral("nodiratime")] = m_CheckNoDirAtime;
		boxOptions()[QStringLiteral("sync")] = m_CheckSync;
		boxOptions()[QStringLiteral("noexec")] = m_CheckNoExec;
		boxOptions()[QStringLiteral("relatime")] = m_CheckRelAtime;

		setupOptions(entry->options);
	}

	if (partition().fileSystem().uuid().isEmpty())
	{
		radioUUID().setEnabled(false);
		if (radioUUID().isChecked())
			radioDeviceNode().setChecked(true);
	}

	if (partition().fileSystem().label().isEmpty())
	{
		radioLabel().setEnabled(false);
		if (radioLabel().isChecked())
			radioDeviceNode().setChecked(true);
	}
}

EditMountPointDialogWidget::~EditMountPointDialogWidget()
{
	qDeleteAll(mountPoints().values());
}

void EditMountPointDialogWidget::setupOptions(const QStringList& options)
{
	QStringList optTmpList;

	foreach (const QString& o, options)
		if (boxOptions().find(o) != boxOptions().end())
			boxOptions()[o]->setChecked(true);
		else
			optTmpList.append(o);

	m_Options = optTmpList.join(QStringLiteral(","));
}

void EditMountPointDialogWidget::on_m_ButtonSelect_clicked(bool)
{
	const QString s = QFileDialog::getExistingDirectory(this, editPath().text());
	if (!s.isEmpty())
		editPath().setText(s);
}

void EditMountPointDialogWidget::on_m_ButtonMore_clicked(bool)
{
	QPointer<EditMountOptionsDialog>  dlg = new EditMountOptionsDialog(this, m_Options.split(QStringLiteral(",")));

	if (dlg->exec() == QDialog::Accepted)
		setupOptions(dlg->options());

	delete dlg;
}

QStringList EditMountPointDialogWidget::options()
{
	QStringList optList = m_Options.split(QStringLiteral(","), QString::SkipEmptyParts);

	foreach (const QString& s, boxOptions().keys())
		if (boxOptions()[s]->isChecked())
			optList.append(s);

	return optList;
}

bool EditMountPointDialogWidget::readMountpoints(const QString& filename)
{
	FILE* fp = setmntent(filename.toLocal8Bit().constData(), "r");

	if (fp == NULL)
	{
		KMessageBox::sorry(this,
				xi18nc("@info", "Could not open mount point file <filename>%1</filename>.", filename),
				i18nc("@title:window", "Error while reading mount points"));
		return false;
	}

	struct mntent* mnt = NULL;

	while ((mnt = getmntent(fp)) != NULL)
	{
		QString device = QString::fromUtf8(mnt->mnt_fsname);
		MountEntry::IdentifyType type = MountEntry::deviceNode;

		if (device.startsWith(QStringLiteral("UUID=")))
		{
			type = MountEntry::uuid;
			device = findBlkIdDevice(QStringLiteral("UUID"), QString(device).remove(QStringLiteral("UUID=")));
		}
		else if (device.startsWith(QStringLiteral("LABEL=")))
		{
			type = MountEntry::label;
			device = findBlkIdDevice(QStringLiteral("LABEL"), QString(device).remove(QStringLiteral("LABEL=")));
		}
		else if (device.startsWith(QStringLiteral("/")))
			device = QFile::symLinkTarget(device);

		if (!device.isEmpty())
		{
			QString mountPoint = QString::fromUtf8(mnt->mnt_dir);
			mountPoints()[device] = new MountEntry(mnt, type);
		}
	}

	endmntent(fp);

	return true;
}

static void writeEntry(QFile& output, const MountEntry* entry)
{
	Q_ASSERT(entry);

	if (entry == NULL)
		return;

	if (entry->path.isEmpty())
		return;

	QTextStream s(&output);

	s << entry->name << "\t"
		<< entry->path << "\t"
		<< entry->type << "\t"
		<< (entry->options.size() > 0 ? entry->options.join(QStringLiteral(",")) : QStringLiteral("defaults")) << "\t"
		<< entry->dumpFreq << "\t"
		<< entry->passNumber << "\n";
}

bool EditMountPointDialogWidget::acceptChanges()
{
	MountEntry* entry = NULL;

	if (mountPoints().find(labelName().text()) == mountPoints().end())
	{
		qWarning() << "could not find device " << labelName().text() << " in mount points.";
		return false;
	}

	entry = mountPoints()[labelName().text()];

	entry->dumpFreq = spinDumpFreq().value();
	entry->passNumber = spinPassNumber().value();
	entry->path = editPath().text();
	entry->options = options();

	if (radioUUID().isChecked() && !partition().fileSystem().uuid().isEmpty())
		entry->name = QStringLiteral("UUID=") + partition().fileSystem().uuid();
	else if (radioLabel().isChecked() && !partition().fileSystem().label().isEmpty())
		entry->name = QStringLiteral("LABEL=") + partition().fileSystem().label();
	else
		entry->name = partition().deviceNode();

	return true;
}

bool EditMountPointDialogWidget::writeMountpoints(const QString& filename)
{
	bool rval = true;
	const QString newFilename = QStringLiteral("%1.new").arg(filename);
	QFile out(newFilename);

	if (!out.open(QFile::ReadWrite | QFile::Truncate))
	{
		qWarning() << "could not open output file " << newFilename;
		rval = false;
	}
	else
	{
		foreach (const MountEntry* me, mountPoints())
			writeEntry(out, me);

		out.close();

		const QString bakFilename = QStringLiteral("%1.bak").arg(filename);
		QFile::remove(bakFilename);

		if (QFile::exists(filename) && !QFile::rename(filename, bakFilename))
		{
			qWarning() << "could not rename " << filename << " to " << bakFilename;
			rval = false;
		}

		if (rval && !QFile::rename(newFilename, filename))
		{
			qWarning() << "could not rename " << newFilename << " to " << filename;
			rval = false;
		}
	}

	if (!rval)
		KMessageBox::sorry(this,
				xi18nc("@info", "Could not save mount points to file <filename>%1</filename>.", filename),
				i18nc("@title:window", "Error While Saving Mount Points"));

	return rval;
}
