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

#ifndef DBSQLPLUGININTERFACE_H
#define DBSQLPLUGININTERFACE_H

// SQL database plugin interface – works with any QIODevice as backing store.

#include "PluginInterface.h"

class QIODevice;
class QSqlDatabase;

#define DBSQLPluginInterface_iid "io.stateoftheart.asocial.plugin.DBSQLPluginInterface"

/**
 * @brief Plugin interface for SQL database providers.
 *
 * The plugin manages an in-memory SQLite database whose serialised form is
 * read from / written to a caller-supplied QIODevice.  This keeps the
 * plugin completely decoupled from the storage layer: the device may be a
 * VirtualFile from an encrypted VFS, a plain QBuffer for testing, a
 * QFile, or anything else that supports read + write + seek.
 *
 * Typical usage (with encrypted VFS):
 * @code
 *   EncContainerPluginInterface* vfs = encPlugin->openContainer("data.vfs", passphrase);
 *   QIODevice* dbfile = vfs->openFile("profiles/main.db");
 *   QSqlDatabase db   = dbsqlPlugin->openDatabaseFile(dbfile);
 *   db.exec("CREATE TABLE IF NOT EXISTS …");
 *   dbsqlPlugin->flushDatabase();        // force-persist to device
 *   dbsqlPlugin->closeDatabase();
 *   dbfile->close();
 *   delete dbfile;
 *   vfs->close();
 *   delete vfs;
 * @endcode
 *
 * Standalone usage (no encryption):
 * @code
 *   QBuffer buf;
 *   buf.open(QIODevice::ReadWrite);
 *   QSqlDatabase db = dbsqlPlugin->openDatabaseFile(&buf);
 *   // …
 *   dbsqlPlugin->closeDatabase();
 * @endcode
 *
 * @note The supplied QIODevice must remain open and accessible for the
 *       lifetime of the database.  The plugin does NOT take ownership.
 * @note Thread safety: all methods must be called from a single thread.
 */
class DBSQLPluginInterface : public PluginInterface
{
public:
    virtual ~DBSQLPluginInterface(){};

    /**
     * @brief Return the plugin-type identifier.
     */
    static QLatin1String type() { return QLatin1String(DBSQLPluginInterface_iid); }

    /**
     * @brief Open (or create) a SQLite database backed by a QIODevice.
     *
     * Existing data in the device is deserialised into a @c :memory: SQLite
     * database.  All subsequent SQL operations happen in RAM.
     *
     * @param device  Readable + writable + seekable QIODevice (not owned).
     * @return A ready-to-use in-memory QSqlDatabase handle.
     */
    virtual QSqlDatabase openDatabaseFile(QIODevice* device) = 0;

    /**
     * @brief Serialise the in-memory database back to the device.
     */
    virtual void flushDatabase() = 0;

    /**
     * @brief Flush and close the database, releasing the device reference.
     */
    virtual void closeDatabase() = 0;
};

Q_DECLARE_INTERFACE(DBSQLPluginInterface, DBSQLPluginInterface_iid)

#endif // DBSQLPLUGININTERFACE_H
