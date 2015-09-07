/***************************************************************************
 *   Copyright (C) 2010 by Volker Lanz <vl@fidra.de                        *
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

#include "backend/corebackendmanager.h"
#include "backend/corebackend.h"

#include <QDebug>
#include <QStringList>
#include <QString>

#include <KAboutData>
#include <KLocalizedString>
#include <KPluginLoader>
#include <KServiceTypeTrader>

#include <config.h>

CoreBackendManager::CoreBackendManager() :
	m_Backend(NULL)
{
}

CoreBackendManager* CoreBackendManager::self()
{
	static CoreBackendManager* instance = NULL;

	if (instance == NULL)
		instance = new CoreBackendManager;

	return instance;
}

KService::List CoreBackendManager::list() const
{
	return KServiceTypeTrader::self()->query(QStringLiteral("PartitionManager/Plugin"), QStringLiteral("[X-KDE-PluginInfo-Category] == 'BackendPlugin'"));
}

bool CoreBackendManager::load(const QString& name)
{
	if (backend())
		unload();

	KPluginLoader loader(name);

	KPluginFactory* factory = loader.factory();

	if (factory != NULL)
	{
		m_Backend = factory->create<CoreBackend>(NULL);
// FIXME: port KF5
//  		backend()->setAboutData(factory->componentData().aboutData());
// 		qDebug() << "Loaded backend plugin: " << backend()->about().displayName() << ", " << backend()->about().version();
		return true;
	}

	qWarning() << "Could not load plugin for core backend " << name << ": " << loader.errorString();
	return false;
}

void CoreBackendManager::unload()
{
	delete m_Backend;
	m_Backend = NULL;
}
