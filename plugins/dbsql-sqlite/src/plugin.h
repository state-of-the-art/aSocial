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

#ifndef DBSQLSQLITEPLUGIN_H
#define DBSQLSQLITEPLUGIN_H

#include <QObject>
#include "plugin/DBSQLPluginInterface.h"
#include "VirtualSqliteDatabase.h"

class Plugin : public QObject, public DBSQLPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "io.stateoftheart.asocial.plugin.DBSQLSqlitePlugin")
    Q_INTERFACES(DBSQLPluginInterface PluginInterface)

public:
    Plugin();
    ~Plugin() override;

    static Plugin* s_pInstance;

    // PluginInterface
    Q_INVOKABLE QString name() const override;
    QStringList requirements() const override;
    bool init() override;
    bool deinit() override;
    bool configure() override;

    // DBSQLPluginInterface
    QSqlDatabase openDatabaseFile(QIODevice* device) override;
    void flushDatabase() override;
    void closeDatabase() override;

signals:
    void appNotice(QString msg) override;
    void appWarning(QString msg) override;
    void appError(QString msg) override;

private:
    VirtualSqliteDatabase m_db;
};

#endif // DBSQLSQLITEPLUGIN_H
