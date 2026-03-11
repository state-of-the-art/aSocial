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

#include "RocksDBStore.h"
#include "Log.h"

#include <QBuffer>
#include <QDataStream>
#include <QDir>
#include <QRandomGenerator>
#include <QTemporaryDir>

#include "rocksdb/db.h"
#include "rocksdb/iterator.h"
#include "rocksdb/options.h"
#include "rocksdb/write_batch.h"

RocksDBStore::RocksDBStore() = default;

RocksDBStore::~RocksDBStore()
{
    if( isOpen() )
        close();
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool RocksDBStore::open(const QString& path)
{
    if( isOpen() )
        close();

    QDir dir(path);
    if( !dir.exists() && !dir.mkpath(".") ) {
        LOG_W() << "Failed to create database directory:" << path;
        return false;
    }

    rocksdb::Options opts;
    opts.create_if_missing = true;
    // Optimise for small-to-medium KV workloads typical of profile JSON
    opts.OptimizeForSmallDb();

    rocksdb::Status st = rocksdb::DB::Open(opts, path.toStdString(), &m_db);
    if( !st.ok() ) {
        LOG_W() << "RocksDB open failed:" << QString::fromStdString(st.ToString());
        m_db = nullptr;
        return false;
    }

    m_device = nullptr;
    return true;
}

bool RocksDBStore::open(QIODevice* device)
{
    if( !device || !device->isOpen() ) {
        LOG_W() << "Device is null or not open";
        return false;
    }

    if( isOpen() )
        close();

    m_tempDir = new QTemporaryDir();
    if( !m_tempDir->isValid() ) {
        LOG_W() << "Failed to create temporary directory";
        delete m_tempDir;
        m_tempDir = nullptr;
        return false;
    }

    rocksdb::Options opts;
    opts.create_if_missing = true;
    opts.OptimizeForSmallDb();

    rocksdb::Status st = rocksdb::DB::Open(opts, m_tempDir->path().toStdString(), &m_db);
    if( !st.ok() ) {
        LOG_W() << "RocksDB temp open failed:" << QString::fromStdString(st.ToString());
        m_db = nullptr;
        delete m_tempDir;
        m_tempDir = nullptr;
        return false;
    }

    m_device = device;

    if( device->size() > 0 ) {
        if( !deserialiseFromDevice() ) {
            LOG_W() << "Failed to deserialise existing data from device";
            delete m_db;
            m_db = nullptr;
            delete m_tempDir;
            m_tempDir = nullptr;
            m_device = nullptr;
            return false;
        }
    }

    return true;
}

bool RocksDBStore::isOpen() const
{
    return m_db != nullptr;
}

void RocksDBStore::flush()
{
    if( !isOpen() )
        return;

    if( m_device ) {
        serialiseToDevice();
    } else {
        // Filesystem mode: force WAL sync
        rocksdb::FlushOptions fo;
        fo.wait = true;
        m_db->Flush(fo);
    }
}

void RocksDBStore::close()
{
    if( !isOpen() )
        return;

    flush();

    delete m_db;
    m_db = nullptr;
    m_device = nullptr;

    if( m_tempDir ) {
        delete m_tempDir;
        m_tempDir = nullptr;
    }
}

// ---------------------------------------------------------------------------
// CRUD
// ---------------------------------------------------------------------------

bool RocksDBStore::put(const QString& key, const QByteArray& value)
{
    if( !isOpen() )
        return false;

    rocksdb::Status st = m_db->Put(
        rocksdb::WriteOptions(),
        rocksdb::Slice(key.toUtf8().constData(), key.toUtf8().size()),
        rocksdb::Slice(value.constData(), value.size()));

    if( !st.ok() ) {
        LOG_W() << "Put failed for key" << key << ":" << QString::fromStdString(st.ToString());
        return false;
    }
    return true;
}

bool RocksDBStore::get(const QString& key, QByteArray& value) const
{
    if( !isOpen() )
        return false;

    std::string result;
    rocksdb::Status st
        = m_db->Get(rocksdb::ReadOptions(), rocksdb::Slice(key.toUtf8().constData(), key.toUtf8().size()), &result);

    if( st.IsNotFound() )
        return false;

    if( !st.ok() ) {
        LOG_W() << "Get failed for key" << key << ":" << QString::fromStdString(st.ToString());
        return false;
    }

    value = QByteArray(result.data(), static_cast<qsizetype>(result.size()));
    return true;
}

bool RocksDBStore::exists(const QString& key) const
{
    if( !isOpen() )
        return false;

    std::string unused;
    rocksdb::Status st
        = m_db->Get(rocksdb::ReadOptions(), rocksdb::Slice(key.toUtf8().constData(), key.toUtf8().size()), &unused);

    return st.ok();
}

bool RocksDBStore::remove(const QString& key)
{
    if( !isOpen() )
        return false;

    rocksdb::Status st
        = m_db->Delete(rocksdb::WriteOptions(), rocksdb::Slice(key.toUtf8().constData(), key.toUtf8().size()));

    if( !st.ok() ) {
        LOG_W() << "Delete failed for key" << key << ":" << QString::fromStdString(st.ToString());
        return false;
    }
    return true;
}

QStringList RocksDBStore::listKeys(const QString& prefix) const
{
    QStringList keys;
    if( !isOpen() )
        return keys;

    const QByteArray prefixBytes = prefix.toUtf8();
    std::unique_ptr<rocksdb::Iterator> it(m_db->NewIterator(rocksdb::ReadOptions()));

    if( prefixBytes.isEmpty() ) {
        it->SeekToFirst();
    } else {
        it->Seek(rocksdb::Slice(prefixBytes.constData(), prefixBytes.size()));
    }

    for( ; it->Valid(); it->Next() ) {
        const rocksdb::Slice k = it->key();
        QByteArray keyBytes(k.data(), static_cast<qsizetype>(k.size()));

        if( !prefixBytes.isEmpty() && !keyBytes.startsWith(prefixBytes) )
            break;

        keys.append(QString::fromUtf8(keyBytes));
        secureWipe(keyBytes);
    }

    return keys;
}

// ---------------------------------------------------------------------------
// Serialisation (QIODevice mode)
// ---------------------------------------------------------------------------

bool RocksDBStore::serialiseToDevice()
{
    if( !m_device || !m_db )
        return false;

    m_device->seek(0);

    QByteArray buffer;
    {
        QDataStream ds(&buffer, QIODevice::WriteOnly);
        ds.setVersion(QDataStream::Qt_6_0);

        ds << STREAM_VERSION;

        // Count entries first
        quint32 count = 0;
        {
            std::unique_ptr<rocksdb::Iterator> it(m_db->NewIterator(rocksdb::ReadOptions()));
            for( it->SeekToFirst(); it->Valid(); it->Next() )
                ++count;
        }
        ds << count;

        // Write entries
        std::unique_ptr<rocksdb::Iterator> it(m_db->NewIterator(rocksdb::ReadOptions()));
        for( it->SeekToFirst(); it->Valid(); it->Next() ) {
            const rocksdb::Slice k = it->key();
            const rocksdb::Slice v = it->value();
            QByteArray keyBytes(k.data(), static_cast<qsizetype>(k.size()));
            QByteArray valBytes(v.data(), static_cast<qsizetype>(v.size()));
            ds << keyBytes << valBytes;
            secureWipe(keyBytes);
            secureWipe(valBytes);
        }
    }

    qint64 written = m_device->write(buffer);
    bool ok = (written == buffer.size());

    // Truncate the device to the new size if it supports it
    if( ok && m_device->size() > written ) {
        // QBuffer and VFS VirtualFile support resize via the parent
        if( auto* buf = qobject_cast<QBuffer*>(m_device) )
            buf->buffer().resize(static_cast<qsizetype>(written));
    }

    secureWipe(buffer);

    if( !ok )
        LOG_W() << "Serialisation write incomplete:" << written << "of" << buffer.size();

    return ok;
}

bool RocksDBStore::deserialiseFromDevice()
{
    if( !m_device || !m_db )
        return false;

    m_device->seek(0);
    QByteArray raw = m_device->readAll();
    if( raw.isEmpty() )
        return true; // Empty device → empty database

    QDataStream ds(raw);
    ds.setVersion(QDataStream::Qt_6_0);

    quint32 version = 0;
    ds >> version;
    if( version != STREAM_VERSION ) {
        LOG_W() << "Unknown stream version:" << version;
        secureWipe(raw);
        return false;
    }

    quint32 count = 0;
    ds >> count;

    rocksdb::WriteBatch batch;
    for( quint32 i = 0; i < count; ++i ) {
        QByteArray key, value;
        ds >> key >> value;

        if( ds.status() != QDataStream::Ok ) {
            LOG_W() << "Deserialisation truncated at entry" << i;
            secureWipe(key);
            secureWipe(value);
            secureWipe(raw);
            return false;
        }

        batch.Put(rocksdb::Slice(key.constData(), key.size()), rocksdb::Slice(value.constData(), value.size()));
        secureWipe(key);
        secureWipe(value);
    }

    secureWipe(raw);

    rocksdb::Status st = m_db->Write(rocksdb::WriteOptions(), &batch);
    if( !st.ok() ) {
        LOG_W() << "Batch write failed:" << QString::fromStdString(st.ToString());
        return false;
    }

    return true;
}

// ---------------------------------------------------------------------------
// Security
// ---------------------------------------------------------------------------

void RocksDBStore::secureWipe(QByteArray& buf)
{
    if( buf.isEmpty() )
        return;

    auto* rng = QRandomGenerator::global();
    auto* data = reinterpret_cast<quint32*>(buf.data());
    const qsizetype words = buf.size() / static_cast<qsizetype>(sizeof(quint32));
    for( qsizetype i = 0; i < words; ++i )
        data[i] = rng->generate();

    // Handle trailing bytes
    const qsizetype tail = buf.size() % static_cast<qsizetype>(sizeof(quint32));
    for( qsizetype i = buf.size() - tail; i < buf.size(); ++i )
        buf[i] = static_cast<char>(rng->generate() & 0xFF);

    buf.clear();
}
