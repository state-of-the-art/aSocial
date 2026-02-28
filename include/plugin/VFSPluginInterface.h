// Copyright (C) 2026  aSocial Developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// Author: Rabit (@rabits)

#ifndef VFSPLUGININTERFACE_H
#define VFSPLUGININTERFACE_H

// Virtual FileSystem plugin interface – factory for encrypted VFS containers.

#include "PluginInterface.h"

class VFSContainerPluginInterface;

#define VFSPluginInterface_iid "io.stateoftheart.asocial.plugin.VFSPluginInterface"

/**
 * @brief Plugin interface for encryption / encrypted-VFS providers.
 *
 * The plugin itself is a lightweight factory: call openContainer() to
 * obtain an VFSContainerPluginInterface handle that owns all the crypto
 * state and file I/O for one container.  Multiple containers may be open
 * simultaneously.
 *
 * Typical usage:
 * @code
 *   VFSContainerPluginInterface* vfs = encPlugin->openContainer("data.vfs", passphrase);
 *   QIODevice* dbfile = vfs->openFile("profiles/main.db");
 *   // … hand dbfile to any component that speaks QIODevice …
 *   dbfile->close();
 *   delete dbfile;
 *   vfs->close();
 *   delete vfs;
 * @endcode
 */
class VFSPluginInterface : public PluginInterface
{
public:
    virtual ~VFSPluginInterface(){};

    /**
     * @brief Return the plugin-type identifier.
     */
    static QLatin1String type() { return QLatin1String(VFSPluginInterface_iid); }

    /**
     * @brief Open or create an encrypted container and return a handle to it.
     *
     * If the file does not exist a fresh container is created (random size,
     * filled with random bytes for plausible deniability).  An empty
     * @p passphrase activates the built-in demo mode.
     *
     * Ownership of the returned object is transferred to the caller.
     *
     * @param containerPath    Filesystem path to the *.vfs file.
     * @param passphrase       User passphrase (empty → demo mode).
     * @param maxContainerSize Optional cap on newly-created container size
     *                         (0 = default, up to 4 GiB / half of free space).
     * @return VFSContainerPluginInterface*, or @c nullptr on failure.
     */
    virtual VFSContainerPluginInterface* openContainer(
        const QString& containerPath, const QString& passphrase, quint64 maxContainerSize = 0)
        = 0;
};

Q_DECLARE_INTERFACE(VFSPluginInterface, VFSPluginInterface_iid)

#endif // VFSPLUGININTERFACE_H
