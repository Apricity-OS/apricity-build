/* === This file is part of Calamares - <http://github.com/calamares> ===
 *
 *   Copyright 2014, Aurélien Gâteau <agateau@kde.org>
 *
 *   Calamares is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Calamares is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Calamares. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef CALAPM_H
#define CALAPM_H

class CoreBackend;

/**
 * Thin wrapper on top of CoreBackendManager. Hides things like initializing the
 * Config instance or instantiating the backend.
 */
namespace CalaPM
{

/**
 * Initialize PartitionManager Config object and load a PartitionManager
 * backend. It loads the "libparted" plugin by default, but this can be
 * overloaded by settings the environment variable CALAPM_BACKEND. Setting it to
 * "dummy" will load the dummy plugin instead.
 *
 * @return true if initialization was successful.
 */
bool init();

}

#endif /* CALAPM_H */
