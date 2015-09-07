/***************************************************************************
 *   Copyright (C) 2008,2011 by Volker Lanz <vl@fidra.de>                  *
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

#include "gui/mainwindow.h"

#include "backend/corebackend.h"
#include "backend/corebackendmanager.h"

#include "util/helpers.h"

#include <QApplication>
#include <QCommandLineParser>

#include <KAboutData>
#include <KMessageBox>
#include <KLocalizedString>

#include <config.h>

int Q_DECL_IMPORT main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	KAboutData *aboutData = createPartitionManagerAboutData();
	KAboutData::setApplicationData(*aboutData);

	QCommandLineParser parser;
	parser.setApplicationDescription( aboutData->shortDescription() );
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addOption( QCommandLineOption( QLatin1Literal("dontsu"), i18nc("@info:shell", "Do not try to gain super user privileges")));
	parser.addOption( QCommandLineOption( QLatin1Literal("advconfig"), i18nc("@info:shell", "Show advanced tab in configuration dialog")));
	parser.addPositionalArgument( QStringLiteral("device"), i18nc("@info:shell", "Device(s) to manage"), QStringLiteral("[device...]") );

	parser.process(app);

	registerMetaTypes();
	if (!checkPermissions())
		return 0;

	Config::instance(QStringLiteral("partitionmanagerrc"));

	if (!loadBackend())
		return 0;

	if (!checkAccessibleDevices())
		return 0;

	MainWindow* mainWindow = new MainWindow();
	mainWindow->show();

	return app.exec();
}
