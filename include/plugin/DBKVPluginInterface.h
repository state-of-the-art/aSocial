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

#ifndef DBKVPLUGININTERFACE_H
#define DBKVPLUGININTERFACE_H

#include "PluginInterface.h"

#include <QStringList>
#include <QtProtobuf/QProtobufMessage>

class QIODevice;

#define DBKVPluginInterface_iid "io.stateoftheart.asocial.plugin.DBKVPluginInterface"

/**
 * @brief Plugin interface for key-value database providers.
 *
 * Supports two modes of operation:
 *
 * - **Filesystem mode**: openDatabase(path) stores data in a directory on
 *   the real filesystem.  Suitable for unencrypted background / relay
 *   operations where VFS is not required.
 *
 * - **VFS mode**: openDatabase(QIODevice*) uses a caller-supplied device as
 *   the durable backing store.  All data is serialised to / from a single
 *   byte stream, making it compatible with encrypted VFS containers.  The
 *   caller retains ownership of the device, which must remain open for the
 *   lifetime of the database.
 *
 * All CRUD methods operate on QProtobufMessage objects, which are serialised
 * to / from the protobuf wire format internally.  This keeps the data layer
 * tightly integrated with the canonical schema defined in proto/asocial/v1/.
 *
 * ### Key Design Convention
 *
 * Keys follow a hierarchical prefix scheme that enables efficient range
 * scans (see RocksDB iterators / prefix seeking):
 *
 * @code
 *   p                                       -> Profile
 *   a/<persona_uid>                         -> Persona
 *   c/<persona_uid>/<contact_uid>           -> Contact
 *   g/<persona_uid>/<group_uid>             -> Group
 *   gm/<group_uid>/<contact_uid>            -> GroupMember
 *   m/<persona_uid>/<created_ms>/<msg_uid>  -> Message (time-ordered)
 *   e/<persona_uid>/<event_uid>             -> Event
 *   pp/<key>                                -> ProfileParam
 * @endcode
 *
 * Typical usage with encrypted VFS (profile storage):
 * @code
 *   VFSContainerPluginInterface* vfs = vfsPlugin->openContainer("data.vfs", pw);
 *   QIODevice* dev = vfs->openFile("dbkv-rocksdb.dat");
 *   dbkvPlugin->openDatabase(dev);
 *   asocial::v1::Profile prof;
 *   prof.setDisplayName("Alice");
 *   dbkvPlugin->storeObject("p", prof);
 *   dbkvPlugin->flushDatabase();
 *   dbkvPlugin->closeDatabase();
 *   dev->close();  delete dev;
 *   vfs->close();  delete vfs;
 * @endcode
 *
 * Standalone usage (no encryption):
 * @code
 *   dbkvPlugin->openDatabase("/var/lib/asocial/relay-db");
 *   dbkvPlugin->storeObject("peer/abc123", peerMap);
 *   dbkvPlugin->closeDatabase();
 * @endcode
 *
 * @note Thread safety: all methods must be called from a single thread.
 */
class DBKVPluginInterface : public PluginInterface
{
public:
    virtual ~DBKVPluginInterface(){};

    /**
     * @brief Return plugin type identifier.
     */
    static QLatin1String type() { return QLatin1String(DBKVPluginInterface_iid); }

    // ---- Database lifecycle ------------------------------------------------

    /**
     * @brief Open (or create) a database at a filesystem directory path.
     *
     * The plugin manages all files inside the directory directly on disk.
     * This mode is appropriate for unencrypted background operations.
     *
     * Calling this while a database is already open will close the
     * previous one first.
     *
     * @param path  Directory path for the database storage.
     * @return true on success, false on failure.
     */
    virtual bool openDatabase(const QString& path) = 0;

    /**
     * @brief Open (or create) a database backed by a QIODevice.
     *
     * Existing data in the device is deserialised into the plugin's
     * internal storage engine.  All subsequent CRUD operations happen
     * in RAM / temporary storage.  Call flushDatabase() to persist
     * changes back to the device.
     *
     * The caller retains ownership of the device, which must stay open
     * and seekable for the database lifetime.
     *
     * @param device  Readable + writable + seekable QIODevice (not owned).
     * @return true on success, false on failure.
     */
    virtual bool openDatabase(QIODevice* device) = 0;

    /**
     * @brief Whether a database is currently open.
     * @return true if a database session is active.
     */
    virtual bool isDatabaseOpen() const = 0;

    /**
     * @brief Persist the current state back to the backing store.
     *
     * For filesystem mode this may be a no-op (the engine persists
     * automatically).  For QIODevice mode this serialises all key-value
     * pairs back to the device.
     */
    virtual void flushDatabase() = 0;

    /**
     * @brief Flush and close the database, releasing internal resources.
     *
     * After this call CRUD methods will fail until openDatabase() is
     * called again.  The caller's QIODevice (if any) is NOT closed.
     */
    virtual void closeDatabase() = 0;

    // ---- CRUD operations ---------------------------------------------------

    /**
     * @brief List keys that start with the given prefix.
     * @param prefix  Key prefix to match (empty string returns all keys).
     * @return Matching keys, sorted lexicographically.
     */
    virtual QStringList listObjects(const QString& prefix) = 0;

    /**
     * @brief Serialise a QProtobufMessage and store it under the given key.
     *
     * The message is serialised to protobuf wire format using
     * QProtobufSerializer.  Sensitive temporary byte arrays are securely
     * wiped before deallocation.
     *
     * @param key      Object key (must not be empty).
     * @param message  Protobuf message to serialise and store.
     * @return true on success, false on failure.
     */
    virtual bool storeObject(const QString& key, const QProtobufMessage& message) = 0;

    /**
     * @brief Retrieve and deserialise a QProtobufMessage for the given key.
     *
     * The caller must supply a concrete protobuf message instance (e.g.
     * @c asocial::v1::Profile) so the deserialiser knows the field layout.
     *
     * @param key      Object key.
     * @param message  Output message populated on success.
     * @return true on success, false if key not found or on error.
     */
    virtual bool retrieveObject(const QString& key, QProtobufMessage& message) = 0;

    /**
     * @brief Check whether a key exists in the database.
     * @param key  Object key.
     * @return true if the key is present.
     */
    virtual bool objectExists(const QString& key) = 0;

    /**
     * @brief Delete the object at the given key.
     * @param key  Object key.
     * @return true on success, false on failure.
     */
    virtual bool deleteObject(const QString& key) = 0;
};

Q_DECLARE_INTERFACE(DBKVPluginInterface, DBKVPluginInterface_iid)

#endif // DBKVPLUGININTERFACE_H
