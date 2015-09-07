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

#include "gui/smartdialog.h"
#include "gui/smartdialogwidget.h"

#include "core/device.h"
#include "core/smartstatus.h"
#include "core/smartattribute.h"

#include "util/helpers.h"
#include "util/htmlreport.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFontDatabase>
#include <QPushButton>
#include <QTemporaryFile>
#include <QTextDocument>
#include <QTextStream>
#include <QTreeWidgetItem>
#include <qglobal.h>

#include <KConfigGroup>
#include <KFormat>
#include <KLocalizedString>
#include <KIconThemes/KIconLoader>
#include <KIO/CopyJob>
#include <KJobUiDelegate>
#include <KMessageBox>

#include <sys/utsname.h>
#include <unistd.h>

#include <config.h>

/** Creates a new SmartDialog
	@param parent pointer to the parent widget
	@param d the Device
*/
SmartDialog::SmartDialog(QWidget* parent, Device& d) :
	QDialog(parent),
	m_Device(d),
	m_DialogWidget(new SmartDialogWidget(this))
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	setLayout(mainLayout);
	mainLayout->addWidget(&dialogWidget());
	setWindowTitle(xi18nc("@title:window", "SMART Properties: <filename>%1</filename>", device().deviceNode()));

	buttonBox = new QDialogButtonBox(this);
	buttonBox->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Close);
	buttonBox->button(QDialogButtonBox::Save)->setText(i18nc("@action:button", "Save SMART Report"));
	buttonBox->button(QDialogButtonBox::Save)->setIcon(QIcon::fromTheme(QStringLiteral("document-save")));
	mainLayout->addWidget(buttonBox);

	setupDialog();
	setupConnections();

	KConfigGroup kcg(KSharedConfig::openConfig(), "smartDialog");
	restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));
}

/** Destroys a SmartDialog */
SmartDialog::~SmartDialog()
{
	KConfigGroup kcg(KSharedConfig::openConfig(), "smartDialog");
	kcg.writeEntry("Geometry", saveGeometry());
}

void SmartDialog::setupDialog()
{
	if (device().smartStatus().isValid())
	{
		if (device().smartStatus().status())
		{
			dialogWidget().statusText().setText(i18nc("@label SMART disk status", "good"));
			dialogWidget().statusIcon().setVisible(false);
		}
		else
		{
			dialogWidget().statusText().setText(i18nc("@label SMART disk status", "BAD"));
			dialogWidget().statusIcon().setPixmap(KIconLoader().loadIcon(QLatin1String("dialog-warning"), KIconLoader::Small));
		}

		dialogWidget().modelName().setText(device().smartStatus().modelName());
		dialogWidget().firmware().setText(device().smartStatus().firmware());
		dialogWidget().serialNumber().setText(device().smartStatus().serial());

		dialogWidget().temperature().setText(SmartStatus::tempToString(device().smartStatus().temp()));
		const QString badSectors = device().smartStatus().badSectors() > 0
				? QLocale().toString(device().smartStatus().badSectors())
				: i18nc("@label SMART number of bad sectors", "none");
		dialogWidget().badSectors().setText(badSectors);
		dialogWidget().poweredOn().setText(KFormat().formatDuration(device().smartStatus().poweredOn()));
		dialogWidget().powerCycles().setText(QLocale().toString(device().smartStatus().powerCycles()));
		dialogWidget().overallAssessment().setText(SmartStatus::overallAssessmentToString(device().smartStatus().overall()));
		dialogWidget().selfTests().setText(SmartStatus::selfTestStatusToString(device().smartStatus().selfTestStatus()));

		dialogWidget().treeSmartAttributes().clear();

		const QFont f = QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont);
		const QString size = f.pixelSize() != -1 ? QStringLiteral("%1px").arg(f.pixelSize()) : QStringLiteral("%1pt").arg(f.pointSize());

		const QString st = QStringLiteral("<span style=\"font-family:%1;font-size:%2;\">").arg(f.family()).arg(size);

		foreach (const SmartAttribute& a, device().smartStatus().attributes())
		{
			QTreeWidgetItem* item = new QTreeWidgetItem(
				QStringList()
					<< QLocale().toString(a.id())
					<< QStringLiteral("<b>%1</b><br/>%2").arg(a.name()).arg(st + a.desc() + QStringLiteral("</span>"))
					<< (a.failureType() == SmartAttribute::PreFailure ? i18nc("@item:intable", "Pre-Failure") : i18nc("@item:intable", "Old-Age"))
					<< (a.updateType() == SmartAttribute::Online ? i18nc("@item:intable", "Online") : i18nc("@item:intable", "Offline"))
					<< QLocale().toString(a.worst())
					<< QLocale().toString(a.current())
					<< QLocale().toString(a.threshold())
					<< a.raw()
					<< a.assessmentToString()
					<< a.value()
			);
			item->setSizeHint(0, QSize(0, 64));
			dialogWidget().treeSmartAttributes().addTopLevelItem(item);
		}
	}
	else
		dialogWidget().statusText().setText(i18nc("@label", "(unknown)"));

	setMinimumSize(dialogWidget().size());
	resize(dialogWidget().size());
}

void SmartDialog::setupConnections()
{
	connect(buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), SLOT(saveSmartReport()));
	connect(buttonBox->button(QDialogButtonBox::Close), SIGNAL(clicked()), SLOT(close()));
}

QString SmartDialog::toHtml() const
{
	QString rval;
	QTextStream s(&rval);

	if (device().smartStatus().status())
		s << HtmlReport::tableLine(i18n("SMART status:"), i18nc("@label SMART disk status", "good"));
	else
		s << HtmlReport::tableLine(i18n("SMART status:"), i18nc("@label SMART disk status", "BAD"));

	const QString badSectors = device().smartStatus().badSectors() > 0
			? QLocale().toString(device().smartStatus().badSectors())
			: i18nc("@label SMART number of bad sectors", "none");

	s << HtmlReport::tableLine(i18n("Model:"), device().smartStatus().modelName())
		<< HtmlReport::tableLine(i18n("Serial number:"), device().smartStatus().serial())
		<< HtmlReport::tableLine(i18n("Firmware revision:"), device().smartStatus().firmware())
		<< HtmlReport::tableLine(i18n("Temperature:"), SmartStatus::tempToString(device().smartStatus().temp()))
		<< HtmlReport::tableLine(i18n("Bad sectors:"), badSectors)
		<< HtmlReport::tableLine(i18n("Powered on for:"), KFormat().formatDuration(device().smartStatus().poweredOn()))
		<< HtmlReport::tableLine(i18n("Power cycles:"), QLocale().toString(device().smartStatus().powerCycles()))
		<< HtmlReport::tableLine(i18n("Self tests:"), SmartStatus::selfTestStatusToString(device().smartStatus().selfTestStatus()))
		<< HtmlReport::tableLine(i18n("Overall assessment:"), SmartStatus::overallAssessmentToString(device().smartStatus().overall()));

	s << "</table><br/>";

	if (device().smartStatus().isValid())
	{

		const QFont f = QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont);
		const QString size = f.pixelSize() != -1 ? QStringLiteral("%1px").arg(f.pixelSize()) : QStringLiteral("%1pt").arg(f.pointSize());

		const QString st = QStringLiteral("<span style=\"font-family:%1;font-size:%2;\">").arg(f.family()).arg(size);

		s << "<table>\n";

		foreach (const SmartAttribute& a, device().smartStatus().attributes())
		{
			s << "<tr>\n";

			s << "<td>" << QLocale().toString(a.id()) << "</td>\n"
				<< "<td>" << QStringLiteral("<b>%1</b><br/>%2").arg(a.name()).arg(st + a.desc() + QStringLiteral("</span>")) << "</td>\n"
				<< "<td>" << (a.failureType() == SmartAttribute::PreFailure ? i18nc("@item:intable", "Pre-Failure") : i18nc("@item:intable", "Old-Age")) << "</td>\n"
				<< "<td>" << (a.updateType() == SmartAttribute::Online ? i18nc("@item:intable", "Online") : i18nc("@item:intable", "Offline")) << "</td>\n"
				<< "<td>" << QLocale().toString(a.worst()) << "</td>\n"
				<< "<td>" << QLocale().toString(a.current()) << "</td>\n"
				<< "<td>" << QLocale().toString(a.threshold()) << "</td>\n"
				<< "<td>" << a.raw() << "</td>\n"
				<< "<td>" << a.assessmentToString() << "</td>\n"
				<< "<td>" << a.value() << "</td>\n";

			s << "</tr>\n";
		}

		s << "</table>\n";
	}
	else
		s << "(unknown)";

	s.flush();

	return rval;
}

void SmartDialog::saveSmartReport()
{
	const QUrl url = QFileDialog::getSaveFileUrl();

	if (url.isEmpty())
		return;

	QTemporaryFile tempFile;

	if (tempFile.open())
	{
		QTextStream s(&tempFile);

		HtmlReport html;

		s << html.header()
			<< toHtml()
			<< html.footer();

		tempFile.close();

		KIO::CopyJob* job = KIO::move(QUrl::fromLocalFile(tempFile.fileName()), url, KIO::HideProgressInfo);
		job->exec();
		if ( job->error() )
			job->ui()->showErrorMessage();
	}
	else
		KMessageBox::sorry(this, xi18nc("@info", "Could not create temporary file when trying to save to <filename>%1</filename>.", url.fileName()), i18nc("@title:window", "Could Not Save SMART Report."));

}
