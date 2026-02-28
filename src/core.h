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

#ifndef CORE_H
#define CORE_H

#include "CoreInterface.h"
#include "plugin/DBKVPluginInterface.h"
#include "plugin/DBSQLPluginInterface.h"
#include <QObject>
#include <QUuid>
#include <QVariantMap>

class QCoreApplication;

class Core : public QObject, public CoreInterface
{
    Q_OBJECT
    Q_INTERFACES(CoreInterface)

public:
    inline static Core* I()
    {
        if( s_pInstance == nullptr )
            s_pInstance = new Core();
        return s_pInstance;
    }
    inline static void destroyI() { delete s_pInstance; }

    // Database management
    DBKVPluginInterface* getDBKV() const override;
    void setDBKVPlugin(const QString& pluginName);
    DBSQLPluginInterface* getDBSQL() const override;
    void setDBSQLPlugin(const QString& pluginName);

    // Profile management
    QString getCurrentProfileId() const override;
    void setCurrentProfileId(const QString& profileId) override;

    // Profile operations
    QString createProfile(const QString& name) override;
    QString importProfile(const QString& serializedData) override;
    QString exportProfile(const QString& profileId) override;
    bool deleteProfile(const QString& profileId) override;
    QStringList listProfiles() override;
    QVariantMap getProfileInfo(const QString& profileId) override;

    // App core functions
    void setApp(QCoreApplication* app);
    void exit() override;

signals:
    void currentProfileChanged(const QString& profileId) override;

private:
    explicit Core(QObject* parent = nullptr);
    ~Core() override;

    static Core* s_pInstance;

    QCoreApplication* m_app;

    DBKVPluginInterface* m_dbkv;
    DBSQLPluginInterface* m_dbsql;

    QString m_currentProfileId;
};

#endif // CORE_H
