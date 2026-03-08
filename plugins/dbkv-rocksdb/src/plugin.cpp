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

#include "plugin.h"

#include <QLoggingCategory>
#include <QRandomGenerator>
#include <QtProtobuf/QProtobufSerializer>

Q_LOGGING_CATEGORY(Drp, "DBKVRocksdbPlugin")

DBKVRocksdbPlugin::DBKVRocksdbPlugin() = default;

DBKVRocksdbPlugin::~DBKVRocksdbPlugin()
{
    if( m_store.isOpen() )
        m_store.close();
}

// ---------------------------------------------------------------------------
// PluginInterface
// ---------------------------------------------------------------------------

QString DBKVRocksdbPlugin::name() const
{
    return QStringLiteral(PLUGIN_NAME);
}

QString DBKVRocksdbPlugin::version() const
{
    return QStringLiteral(PLUGIN_VERSION);
}

PluginPermissions DBKVRocksdbPlugin::requiredPermissions() const
{
    // Pure storage engine — no Core access needed
    return PluginPermission::None;
}

QStringList DBKVRocksdbPlugin::requirements() const
{
    return {};
}

bool DBKVRocksdbPlugin::init()
{
    if( isInitialized() )
        return true;

    qCDebug(Drp) << "Initializing plugin";

    setInitialized(true);
    emit appNotice(name() + QLatin1String(" initialized"));
    return true;
}

bool DBKVRocksdbPlugin::deinit()
{
    if( !isInitialized() )
        return true;

    qCDebug(Drp) << "Deinitializing plugin";
    closeDatabase();
    setInitialized(false);
    emit appNotice(name() + QLatin1String(" deinitialized"));
    return true;
}

bool DBKVRocksdbPlugin::configure()
{
    qCDebug(Drp) << "Configuring plugin";
    return true;
}

// ---------------------------------------------------------------------------
// Database lifecycle
// ---------------------------------------------------------------------------

bool DBKVRocksdbPlugin::openDatabase(const QString& path)
{
    if( m_store.isOpen() )
        m_store.close();

    if( !m_store.open(path) ) {
        qCWarning(Drp) << "Failed to open database at" << path;
        return false;
    }

    qCDebug(Drp) << "Database opened at" << path;
    return true;
}

bool DBKVRocksdbPlugin::openDatabase(QIODevice* device)
{
    if( m_store.isOpen() )
        m_store.close();

    if( !m_store.open(device) ) {
        qCWarning(Drp) << "Failed to open database from QIODevice";
        return false;
    }

    qCDebug(Drp) << "Database opened from QIODevice";
    return true;
}

bool DBKVRocksdbPlugin::isDatabaseOpen() const
{
    return m_store.isOpen();
}

void DBKVRocksdbPlugin::flushDatabase()
{
    m_store.flush();
}

void DBKVRocksdbPlugin::closeDatabase()
{
    m_store.close();
    qCDebug(Drp) << "Database closed";
}

// ---------------------------------------------------------------------------
// CRUD (protobuf-native)
// ---------------------------------------------------------------------------

QStringList DBKVRocksdbPlugin::listObjects(const QString& prefix)
{
    if( !m_store.isOpen() ) {
        qCWarning(Drp) << "Database not open";
        return {};
    }

    return m_store.listKeys(prefix);
}

bool DBKVRocksdbPlugin::storeObject(const QString& key, const QProtobufMessage& message)
{
    if( !m_store.isOpen() ) {
        qCWarning(Drp) << "Database not open";
        return false;
    }

    QProtobufSerializer serializer;
    QByteArray data = message.serialize(&serializer);
    if( data.isEmpty() && serializer.lastError() != QAbstractProtobufSerializer::Error::None ) {
        qCWarning(Drp) << "Protobuf serialisation failed for key:" << key;
        return false;
    }

    bool ok = m_store.put(key, data);
    secureWipe(data);
    return ok;
}

bool DBKVRocksdbPlugin::retrieveObject(const QString& key, QProtobufMessage& message)
{
    if( !m_store.isOpen() ) {
        qCWarning(Drp) << "Database not open";
        return false;
    }

    QByteArray data;
    if( !m_store.get(key, data) )
        return false;

    QProtobufSerializer serializer;
    bool ok = message.deserialize(&serializer, data);
    secureWipe(data);

    if( !ok )
        qCWarning(Drp) << "Protobuf deserialisation failed for key:" << key;

    return ok;
}

bool DBKVRocksdbPlugin::objectExists(const QString& key)
{
    if( !m_store.isOpen() )
        return false;

    return m_store.exists(key);
}

bool DBKVRocksdbPlugin::deleteObject(const QString& key)
{
    if( !m_store.isOpen() ) {
        qCWarning(Drp) << "Database not open";
        return false;
    }

    return m_store.remove(key);
}

// ---------------------------------------------------------------------------
// Security
// ---------------------------------------------------------------------------

void DBKVRocksdbPlugin::secureWipe(QByteArray& buf)
{
    if( buf.isEmpty() )
        return;

    auto* rng = QRandomGenerator::global();
    auto* data = reinterpret_cast<quint32*>(buf.data());
    const qsizetype words = buf.size() / static_cast<qsizetype>(sizeof(quint32));
    for( qsizetype i = 0; i < words; ++i )
        data[i] = rng->generate();

    const qsizetype tail = buf.size() % static_cast<qsizetype>(sizeof(quint32));
    for( qsizetype i = buf.size() - tail; i < buf.size(); ++i )
        buf[i] = static_cast<char>(rng->generate() & 0xFF);

    buf.clear();
}
