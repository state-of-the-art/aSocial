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

#ifndef DBKVJSONPLUGIN_H
#define DBKVJSONPLUGIN_H

#include <QDir>
#include <QObject>

#include "plugin/DBKVPluginInterface.h"

/**
 * @brief Simple file-based key-value plugin storing protobuf wire data.
 *
 * Each key is stored as a separate binary file inside a data directory.
 * The file contents are raw protobuf wire format bytes.
 *
 * This plugin is intended for unencrypted background / relay operations.
 * QIODevice mode is not supported -- openDatabase(QIODevice*) always
 * returns false.  Use dbkv-rocksdb for VFS-backed profile storage.
 */
class DBKVJsonPlugin : public QObject, public DBKVPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "io.stateoftheart.asocial.plugin.DBKVJsonPlugin")
    Q_INTERFACES(DBKVPluginInterface PluginInterface)

public:
    DBKVJsonPlugin();
    ~DBKVJsonPlugin() override;

    // PluginInterface
    Q_INVOKABLE QString name() const override;
    QString version() const override;
    PluginPermissions requiredPermissions() const override;
    QStringList requirements() const override;
    bool init() override;
    bool deinit() override;
    bool configure() override;

    // DBKVPluginInterface - database lifecycle
    bool openDatabase(const QString& path) override;
    bool openDatabase(QIODevice* device) override;
    bool isDatabaseOpen() const override;
    void flushDatabase() override;
    void closeDatabase() override;

    // DBKVPluginInterface - CRUD (protobuf-native)
    QStringList listObjects(const QString& prefix) override;
    bool storeObject(const QString& key, const QProtobufMessage& message) override;
    bool retrieveObject(const QString& key, QProtobufMessage& message) override;
    bool objectExists(const QString& key) override;
    bool deleteObject(const QString& key) override;

signals:
    void appNotice(QString msg) override;
    void appWarning(QString msg) override;
    void appError(QString msg) override;

private:
    /** @brief Overwrite @p buf with random bytes, then clear it. */
    static void secureWipe(QByteArray& buf);

    QDir m_dataDir;
    bool m_dbOpen = false;

    /**
     * @brief Map a hierarchical key to a filesystem path.
     *
     * Forward slashes in the key become directory separators so that
     * prefix-based listing maps naturally to directory traversal.
     */
    QString getObjectFilePath(const QString& key) const;
};

#endif // DBKVJSONPLUGIN_H
