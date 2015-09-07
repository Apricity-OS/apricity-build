/***************************************************************************
 *   Copyright (C) 2008, 2009, 2010 by Volker Lanz <vl@fidra.de>           *
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

#include "util/helpers.h"
#include "util/globallog.h"

#include "backend/corebackendmanager.h"

#include "ops/operation.h"

#include <KAboutData>
#include <KMessageBox>
#include <KLocalizedString>

#include <Solid/Device>

#include <QAction>
#include <QApplication>
#include <QCollator>
#include <QFileInfo>
#include <QIcon>
#include <QMenu>
#include <QHeaderView>
#include <QPainter>
#include <QPixmap>
#include <QProcess>
#include <QStandardPaths>
#include <QRect>
#include <QTreeWidget>

#include <config.h>

#include <unistd.h>
#include <signal.h>

void registerMetaTypes()
{
	qRegisterMetaType<Operation*>("Operation*");
	qRegisterMetaType<Log::Level>("Log::Level");
}

static QString suCommand()
{
	const QString candidates[] = { QStringLiteral("kdesu"), QStringLiteral("kdesudo"), QStringLiteral("gksudo"), QStringLiteral("gksu") };
	QString rval;

	for (quint32 i = 0; i < sizeof(candidates) / sizeof(candidates[0]); i++)
	{
		rval = QStandardPaths::findExecutable(candidates[i]);
		if (QFileInfo(rval).isExecutable())
			return rval;
	}

	return QString();
}

bool checkPermissions()
{
	if (geteuid() != 0)
	{
		// only try to gain root privileges if we have a valid (kde|gk)su(do) command and
		// we did not try so before: the dontsu-option is there to make sure there are no
		// endless loops of calling the same non-working (kde|gk)su(do) binary again and again.
		if (!suCommand().isEmpty() && !QCoreApplication::arguments().contains(QLatin1String("--dontsu")))
		{
			QStringList argList;

			const QString suCmd = suCommand();

			// kdesu broke backward compatibility at some point and now only works with "-c";
			// kdesudo accepts either (with or without "-c"), but the gk* helpers only work
			// without. kdesu maintainers won't fix their app, so we need to work around that here.
			if (suCmd.indexOf(QStringLiteral("kdesu")) != -1)
				argList << QStringLiteral("-c");

			argList << QCoreApplication::arguments() << QStringLiteral(" --dontsu");

			if (QProcess::execute(suCmd, argList) == 0)
				return false;
		}

		return KMessageBox::warningContinueCancel(NULL, xi18nc("@info",
				"<para><warning>You do not have administrative privileges.</warning></para>"
				"<para>It is possible to run <application>%1</application> without these privileges. "
				"You will, however, <emphasis>not</emphasis> be allowed to apply operations.</para>"
				"<para>Do you want to continue running <application>%1</application>?</para>",
				QGuiApplication::applicationDisplayName()),
	 		i18nc("@title:window", "No administrative privileges"),
			KGuiItem(i18nc("@action:button", "Run without administrative privileges"), QStringLiteral("arrow-right")),
			KStandardGuiItem::cancel(),
			QStringLiteral("runWithoutRootPrivileges")) == KMessageBox::Continue;
	}

	return true;
}

KAboutData* createPartitionManagerAboutData()
{
	KAboutData* about = new KAboutData(
		QStringLiteral("partitionmanager"),
		xi18nc("@title", "<application>KDE Partition Manager</application>"),
		QStringLiteral(VERSION),
		i18nc("@title", "Manage your disks, partitions and file systems"),
		KAboutLicense::GPL,
		i18nc("@info:credit", "© 2008-2013 Volker Lanz"));
	about->setOrganizationDomain(QByteArray("kde.org"));
	about->setProgramIconName(QStringLiteral("partitionmanager"));
	about->setProductName(QByteArray("partitionmanager"));

	about->addAuthor(i18nc("@info:credit", "Volker Lanz"), i18nc("@info:credit", "Former maintainer"));
	about->addAuthor(i18nc("@info:credit", "Andrius Štikonas"), i18nc("@info:credit", "Developer"), QStringLiteral("andrius@stikonas.eu"));
	about->setHomepage(QStringLiteral("http://www.partitionmanager.org"));

	about->addCredit(i18n("Hugo Pereira Da Costa"), i18nc("@info:credit", "Partition Widget Design"), QStringLiteral("hugo@oxygen-icons.org"));

	return about;
}

bool caseInsensitiveLessThan(const QString& s1, const QString& s2)
{
	return s1.toLower() < s2.toLower();
}

bool naturalLessThan(const QString& s1, const QString& s2)
{
	QCollator c;
	c.setNumericMode(true);
	c.setCaseSensitivity(Qt::CaseSensitive);
	return c.compare(s1, s2) < 0;
}

QIcon createFileSystemColor(FileSystem::Type type, quint32 size)
{
	QPixmap pixmap(size, size);
	QPainter painter(&pixmap);
	painter.setPen(QColor(0, 0, 0));
	painter.setBrush(Config::fileSystemColorCode(type));
	painter.drawRect(QRect(0, 0, pixmap.width() - 1, pixmap.height() - 1));
	painter.end();

	return QIcon(pixmap);
}

void showColumnsContextMenu(const QPoint& p, QTreeWidget& tree)
{
	QMenu headerMenu(i18nc("@title:menu", "Columns"));

	QHeaderView* header = tree.header();

	for (qint32 i = 0; i < tree.model()->columnCount(); i++)
	{
		const int idx = header->logicalIndex(i);
		const QString text = tree.model()->headerData(idx, Qt::Horizontal).toString();

		QAction* action = headerMenu.addAction(text);
		action->setCheckable(true);
		action->setChecked(!header->isSectionHidden(idx));
		action->setData(idx);
		action->setEnabled(idx > 0);
	}

	QAction* action = headerMenu.exec(tree.header()->mapToGlobal(p));

	if (action != NULL)
	{
		const bool hidden = !action->isChecked();
		tree.setColumnHidden(action->data().toInt(), hidden);
		if (!hidden)
			tree.resizeColumnToContents(action->data().toInt());
	}
}

bool loadBackend()
{
	if (CoreBackendManager::self()->load(Config::backend()) == false)
	{
		if (CoreBackendManager::self()->load(CoreBackendManager::defaultBackendName()))
		{
			KMessageBox::sorry(NULL,
				xi18nc("@info", "<para>The configured backend plugin \"%1\" could not be loaded.</para>"
					"<para>Loading the default backend plugin \"%2\" instead.</para>",
				Config::backend(), CoreBackendManager::defaultBackendName()),
				i18nc("@title:window", "Error: Could Not Load Backend Plugin"));
			Config::setBackend(CoreBackendManager::defaultBackendName());
		}
		else
		{
			KMessageBox::error(NULL,
				xi18nc("@info", "<para>Neither the configured (\"%1\") nor the default (\"%2\") backend "
					"plugin could be loaded.</para><para>Please check your installation.</para>",
				Config::backend(), CoreBackendManager::defaultBackendName()),
				i18nc("@title:window", "Error: Could Not Load Backend Plugin"));
			return false;
		}
	}

	return true;
}

bool checkAccessibleDevices()
{
	if (getSolidDeviceList().empty())
	{
		KMessageBox::error(NULL,
			xi18nc("@info", "<para>No usable devices could be found.</para><para>Make sure you have sufficient "
				"privileges to access block devices on your system.</para>"),
			i18nc("@title:window", "Error: No Usable Devices Found"));
		return false;
	}

	return true;
}

QList<Solid::Device> getSolidDeviceList()
{
#ifdef ENABLE_UDISKS2
        QString predicate = QStringLiteral("StorageVolume.usage == 'PartitionTable'");

#else
        QString predicate = QStringLiteral("[ [ [ StorageDrive.driveType == 'HardDisk' OR StorageDrive.driveType == 'CompactFlash'] OR "
                "[ StorageDrive.driveType == 'MemoryStick' OR StorageDrive.driveType == 'SmartMedia'] ] OR "
                "[ StorageDrive.driveType == 'SdMmc' OR StorageDrive.driveType == 'Xd'] ]");
#endif

	QStringList argList;
	int argc = argList.size();
	if (argc > 0)
	{
		predicate = QStringLiteral(" [ ") + predicate + QStringLiteral(" AND ");

		qint32 brackets = (argc + 1) / 2;
		brackets = argc == 1 ? 0 : brackets;
		for (qint32 i = 0; i < brackets; i++)
			predicate += QStringLiteral("[ ");

		bool right_bracket = false;
		for (qint32 i = 0; i < argc; i++, right_bracket =! right_bracket)
		{
			predicate += QStringLiteral("Block.device == '%1' ").arg(argList[i]);

			if (right_bracket)
				predicate += i == 1 ? QStringLiteral("] ") : QStringLiteral("] ] ");
			if (i < argc - 1)
				predicate += QStringLiteral("OR ");
			if (right_bracket && i != argc - 2 && i != argc - 1)
				predicate += QStringLiteral("[ ");
		}
		predicate += right_bracket && brackets > 0 ? QStringLiteral("] ]") : QStringLiteral("]");
	}

	return Solid::Device::listFromQuery(predicate);
}
