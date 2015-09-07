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

#include "plugins/dummy/dummybackend.h"
#include "plugins/libparted/libpartedbackend.h"

#include <QDebug>
#include <QString>

/**
 * Simplified implementation which assumes the code for plugine is built inside
 * the library itself. Using plugins this way instead of using KService has the
 * following benefits:
 *
 * - No dependency on KService
 * - Avoid having to modify the plugin install name and desktop files to avoid
 *   clashes with an existing installation of partitionmanager
 */

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

bool CoreBackendManager::load(const QString& name)
{
	if (backend())
		unload();

	m_Backend = 0;
	if (name == "libparted")
		m_Backend = new LibPartedBackend(0, QVariantList());
	else if (name == "dummy")
		m_Backend = new DummyBackend(0, QVariantList());

	if (m_Backend != NULL)
	{
		return true;
	}

	qWarning() << "No plugin named" << name;
	return false;
}

void CoreBackendManager::unload()
{
	delete m_Backend;
	m_Backend = NULL;
}

