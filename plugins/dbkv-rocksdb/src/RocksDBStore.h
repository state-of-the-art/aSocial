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

#ifndef ROCKSDBSTORE_H
#define ROCKSDBSTORE_H

#include <QByteArray>
#include <QIODevice>
#include <QString>
#include <QStringList>

#include "rocksdb/db.h"

class QTemporaryDir;

/**
 * @brief Low-level RocksDB wrapper with dual backing-store support.
 *
 * Provides two modes of operation:
 *
 * 1. **Filesystem mode** – open(path) creates / opens a RocksDB database
 *    at the given directory.  All writes are persisted by RocksDB itself.
 *
 * 2. **QIODevice mode** – open(QIODevice*) deserialises key-value pairs
 *    from the device into a temporary RocksDB.  flush() serialises the
 *    current snapshot back.  The temporary directory is cleaned up on
 *    close().
 *
 * The serialisation format (QIODevice mode) is a simple QDataStream:
 * @code
 *   quint32  version   (currently 1)
 *   quint32  count     (number of KV pairs)
 *   For each pair:
 *     QByteArray  key
 *     QByteArray  value
 * @endcode
 *
 * Sensitive temporaries (QByteArrays holding deserialised data) are
 * overwritten with random bytes before deallocation.
 */
class RocksDBStore
{
public:
    RocksDBStore();
    ~RocksDBStore();

    RocksDBStore(const RocksDBStore&) = delete;
    RocksDBStore& operator=(const RocksDBStore&) = delete;

    /**
     * @brief Open a RocksDB database at the given directory path.
     *
     * Creates the directory (and any parents) if it does not exist.
     * If a database is already open it is closed first.
     *
     * @param path  Filesystem directory for RocksDB files.
     * @return true on success.
     */
    bool open(const QString& path);

    /**
     * @brief Open a RocksDB database backed by a QIODevice.
     *
     * Reads the device, deserialises key-value pairs into a temporary
     * on-disk RocksDB, and uses that for all subsequent operations.
     *
     * @param device  Readable + writable + seekable QIODevice (not owned).
     * @return true on success.
     */
    bool open(QIODevice* device);

    /**
     * @brief Whether the database is currently open.
     */
    bool isOpen() const;

    /**
     * @brief Persist all data back to the backing store.
     *
     * Filesystem mode: no-op (RocksDB WAL handles durability).
     * QIODevice mode : serialises all KV pairs to the device.
     */
    void flush();

    /**
     * @brief Flush and close the database, freeing all resources.
     */
    void close();

    /**
     * @brief Store a raw byte value under the given key.
     * @return true on success.
     */
    bool put(const QString& key, const QByteArray& value);

    /**
     * @brief Retrieve the raw bytes stored under the given key.
     * @param[out] value  Populated on success.
     * @return true if found, false otherwise.
     */
    bool get(const QString& key, QByteArray& value) const;

    /**
     * @brief Check whether the given key exists.
     */
    bool exists(const QString& key) const;

    /**
     * @brief Delete the value at the given key.
     * @return true on success (including key-not-found).
     */
    bool remove(const QString& key);

    /**
     * @brief List all keys that start with @p prefix.
     * @param prefix  Key prefix (empty = all keys).
     * @return Sorted list of matching keys.
     */
    QStringList listKeys(const QString& prefix) const;

private:
    static constexpr quint32 STREAM_VERSION = 1;

    /** @brief Overwrite @p buf with random bytes, then clear it. */
    static void secureWipe(QByteArray& buf);

    /** @brief Serialise all KV pairs to the device. */
    bool serialiseToDevice();

    /** @brief Deserialise KV pairs from the device into the open DB. */
    bool deserialiseFromDevice();

    rocksdb::DB* m_db = nullptr;
    QIODevice* m_device = nullptr;      ///< Non-owning; set only in QIODevice mode
    QTemporaryDir* m_tempDir = nullptr; ///< Temp directory for QIODevice mode
};

#endif // ROCKSDBSTORE_H
