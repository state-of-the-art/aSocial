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

#include "Log.h"
#include <QByteArray>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QRandomGenerator>
#include <QtProtobuf/QProtobufSerializer>

DBKVJsonPlugin::DBKVJsonPlugin() {}

DBKVJsonPlugin::~DBKVJsonPlugin() {}

QString DBKVJsonPlugin::name() const
{
    return QStringLiteral(PLUGIN_NAME);
}

QString DBKVJsonPlugin::version() const
{
    return QStringLiteral(PLUGIN_VERSION);
}

PluginPermissions DBKVJsonPlugin::requiredPermissions() const
{
    // Reads workdir.appdata to locate its data directory
    return PluginPermission::SettingsRead;
}

QStringList DBKVJsonPlugin::requirements() const
{
    return QStringList();
}

bool DBKVJsonPlugin::init()
{
    LOG_D() << "Initializing plugin";

    setInitialized(true);
    return true;
}

bool DBKVJsonPlugin::deinit()
{
    LOG_D() << "Deinitializing plugin";
    closeDatabase();
    setInitialized(false);
    return true;
}

bool DBKVJsonPlugin::configure()
{
    LOG_D() << "Configuring plugin";
    return true;
}

// ---------------------------------------------------------------------------
// Database lifecycle
// ---------------------------------------------------------------------------

bool DBKVJsonPlugin::openDatabase(const QString& path)
{
    if( m_dbOpen )
        closeDatabase();

    m_dataDir = QDir(path);

    if( !m_dataDir.exists() ) {
        if( !m_dataDir.mkpath(".") ) {
            LOG_W() << "Failed to create data directory:" << m_dataDir.absolutePath();
            return false;
        }
    }

    m_dbOpen = true;
    LOG_D() << "Database opened at:" << m_dataDir.absolutePath();
    return true;
}

bool DBKVJsonPlugin::openDatabase(QIODevice* /*device*/)
{
    LOG_W() << "QIODevice mode is not supported by the JSON plugin." << "Use dbkv-rocksdb for VFS-backed storage.";
    return false;
}

bool DBKVJsonPlugin::isDatabaseOpen() const
{
    return m_dbOpen;
}

void DBKVJsonPlugin::flushDatabase()
{
    // No-op: each storeObject() writes directly to disk
}

void DBKVJsonPlugin::closeDatabase()
{
    m_dbOpen = false;
    LOG_D() << "Database closed";
}

// ---------------------------------------------------------------------------
// CRUD (protobuf-native)
// ---------------------------------------------------------------------------

QStringList DBKVJsonPlugin::listObjects(const QString& prefix)
{
    if( !m_dbOpen ) {
        LOG_W() << "Database not open";
        return {};
    }

    QStringList result;
    const QString suffix = QStringLiteral(".pb");

    QDirIterator it(m_dataDir.absolutePath(), {"*" + suffix}, QDir::Files, QDirIterator::Subdirectories);
    while( it.hasNext() ) {
        it.next();
        // Reconstruct the key from the relative path minus the .pb extension
        QString rel = m_dataDir.relativeFilePath(it.filePath());
        rel.chop(suffix.size());
        if( prefix.isEmpty() || rel.startsWith(prefix) )
            result.append(rel);
    }

    result.sort();
    return result;
}

bool DBKVJsonPlugin::storeObject(const QString& key, const QProtobufMessage& message)
{
    if( !m_dbOpen ) {
        LOG_W() << "Database not open";
        return false;
    }

    QProtobufSerializer serializer;
    QByteArray data = message.serialize(&serializer);
    if( data.isEmpty() && serializer.lastError() != QAbstractProtobufSerializer::Error::None ) {
        LOG_W() << "Protobuf serialisation failed for key:" << key;
        return false;
    }

    QString filePath = getObjectFilePath(key);

    // Ensure parent directories exist
    QFileInfo fi(filePath);
    if( !fi.dir().exists() )
        fi.dir().mkpath(".");

    QFile file(filePath);
    if( !file.open(QIODevice::WriteOnly) ) {
        LOG_W() << "Failed to open file for writing:" << filePath;
        secureWipe(data);
        return false;
    }

    file.write(data);
    file.close();
    secureWipe(data);
    return true;
}

bool DBKVJsonPlugin::retrieveObject(const QString& key, QProtobufMessage& message)
{
    if( !m_dbOpen ) {
        LOG_W() << "Database not open";
        return false;
    }

    QString filePath = getObjectFilePath(key);
    QFile file(filePath);
    if( !file.open(QIODevice::ReadOnly) ) {
        LOG_W() << "Failed to open file for reading:" << filePath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QProtobufSerializer serializer;
    bool ok = message.deserialize(&serializer, data);
    secureWipe(data);

    if( !ok )
        LOG_W() << "Protobuf deserialisation failed for key:" << key;

    return ok;
}

bool DBKVJsonPlugin::objectExists(const QString& key)
{
    if( !m_dbOpen )
        return false;

    QString filePath = getObjectFilePath(key);
    return QFile::exists(filePath);
}

bool DBKVJsonPlugin::deleteObject(const QString& key)
{
    if( !m_dbOpen ) {
        LOG_W() << "Database not open";
        return false;
    }

    QString filePath = getObjectFilePath(key);
    if( QFile::exists(filePath) )
        return QFile::remove(filePath);

    return true;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

QString DBKVJsonPlugin::getObjectFilePath(const QString& key) const
{
    // Key hierarchy maps directly to filesystem directories.
    // e.g. key "a/some-uuid" -> "<datadir>/a/some-uuid.pb"
    return m_dataDir.absoluteFilePath(key + ".pb");
}

// ---------------------------------------------------------------------------
// Security
// ---------------------------------------------------------------------------

void DBKVJsonPlugin::secureWipe(QByteArray& buf)
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
