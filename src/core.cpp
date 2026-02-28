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

#include "core.h"
#include "CoreInterface.h"
#include "plugins.h"
#include "settings.h"

#include <QCoreApplication>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(Cc, "Core")

Core* Core::s_pInstance = nullptr;

Core::Core(QObject* parent)
    : QObject(parent)
    , m_dbkv(nullptr)
    , m_dbsql(nullptr)
{
    qCDebug(Cc) << "Core singleton created";
}

Core::~Core()
{
    qCDebug(Cc) << "Core singleton destroyed";
}

DBKVPluginInterface* Core::getDBKV() const
{
    return m_dbkv;
}

DBSQLPluginInterface* Core::getDBSQL() const
{
    return m_dbsql;
}

void Core::setDBKVPlugin(const QString& pluginName)
{
    // Enable KV database plugin
    Settings::I()->setting("plugins.dbkv.json.active", true);
    Plugins::I()->settingActivePlugin("plugins.dbkv.json.active", "dbkv-json");
    Plugins::I()->activateInterface("dbkv-json", QLatin1String("io.stateoftheart.asocial.plugin.DBKVPluginInterface"));

    m_dbkv = qobject_cast<DBKVPluginInterface*>(
        Plugins::I()->getPlugin("io.stateoftheart.asocial.plugin.DBKVPluginInterface", pluginName));

    if( !m_dbkv ) {
        qCWarning(Cc) << "Failed to load DBKV plugin:" << pluginName;
    } else {
        qCDebug(Cc) << "DBKV plugin set to:" << pluginName;
    }
}

void Core::setDBSQLPlugin(const QString& pluginName)
{
    // Enable SQL database plugin
    Settings::I()->setting("plugins.dbsql.sqlite.active", true);
    Plugins::I()->settingActivePlugin("plugins.dbsql.sqlite.active", "dbsql-sqlite");
    Plugins::I()
        ->activateInterface("dbsql-sqlite", QLatin1String("io.stateoftheart.asocial.plugin.DBSQLPluginInterface"));

    m_dbsql = qobject_cast<DBSQLPluginInterface*>(
        Plugins::I()->getPlugin("io.stateoftheart.asocial.plugin.DBSQLPluginInterface", pluginName));

    if( !m_dbsql ) {
        qCWarning(Cc) << "Failed to load DBSQL plugin:" << pluginName;
    } else {
        qCDebug(Cc) << "DBSQL plugin set to:" << pluginName;
    }
}

QString Core::getCurrentProfileId() const
{
    return m_currentProfileId;
}

void Core::setCurrentProfileId(const QString& profileId)
{
    if( m_currentProfileId != profileId ) {
        m_currentProfileId = profileId;
        emit currentProfileChanged(profileId);
        qCDebug(Cc) << "Current profile changed to:" << profileId;
    }
}

QString Core::createProfile(const QString& name)
{
    if( !m_dbkv ) {
        qCWarning(Cc) << "No DBKV plugin available";
        return QString();
    }

    if( !m_dbsql ) {
        qCWarning(Cc) << "No DBSQL plugin available";
        return QString();
    }

    QUuid profileId = QUuid::createUuid();
    QString profileIdStr = profileId.toString(QUuid::WithoutBraces);

    QVariantMap profileData;
    profileData["id"] = profileIdStr;
    profileData["name"] = name;
    profileData["created"] = QDateTime::currentDateTime();

    // TODO: Move to DBSQL
    if( !m_dbkv->storeObject("profile_" + profileIdStr, profileData) ) {
        qCWarning(Cc) << "Failed to create profile storage object";
        return QString();
    }

    qCDebug(Cc) << "Created profile:" << profileIdStr << "with name:" << name;

    return profileIdStr;
}

QString Core::importProfile(const QString& serializedData)
{
    if( !m_dbkv ) {
        qCWarning(Cc) << "No DBKV plugin available";
        return QString();
    }

    if( !m_dbsql ) {
        qCWarning(Cc) << "No DBSQL plugin available";
        return QString();
    }

    QByteArray jsonData = QByteArray::fromBase64(serializedData.toUtf8());
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);

    if( !doc.isObject() ) {
        qCWarning(Cc) << "Invalid serialized profile data";
        return QString();
    }

    QVariantMap profileData = doc.object().toVariantMap();
    QString profileId = profileData["id"].toString();

    if( profileId.isEmpty() ) {
        qCWarning(Cc) << "Profile data missing ID";
        return QString();
    }

    // TODO: Move to DBSQL
    // Check if profile already exists
    if( m_dbkv->objectExists("profile_" + profileId) ) {
        qCWarning(Cc) << "Profile already exists:" << profileId;
        return QString();
    }

    // Store profile metadata
    if( !m_dbkv->storeObject("profile_" + profileId, profileData) ) {
        qCWarning(Cc) << "Unable to store profile:" << profileId;
        return QString();
    }

    qCDebug(Cc) << "Imported profile:" << profileId;

    return profileId;
}

QString Core::exportProfile(const QString& profileId)
{
    QByteArray jsonData = QJsonDocument::fromVariant(getProfileInfo(profileId)).toJson();

    return QString(jsonData.toBase64());
}

bool Core::deleteProfile(const QString& profileId)
{
    if( !m_dbkv ) {
        qCWarning(Cc) << "No DBKV plugin available";
        return false;
    }

    if( !m_dbsql ) {
        qCWarning(Cc) << "No DBSQL plugin available";
        return false;
    }

    // TODO: Move to DBSQL
    if( m_dbkv->objectExists("profile_" + profileId) ) {
        if( !m_dbkv->deleteObject("profile_" + profileId) ) {
            qCWarning(Cc) << "Failed to delete profile:" << profileId;
            return false;
        }
    }

    qCDebug(Cc) << "Deleted profile:" << profileId;
    return true;
}

QStringList Core::listProfiles()
{
    if( !m_dbkv ) {
        qCWarning(Cc) << "No DBKV plugin available";
        return QStringList();
    }

    if( !m_dbsql ) {
        qCWarning(Cc) << "No DBSQL plugin available";
        return QStringList();
    }

    // TODO: Move to DBSQL
    return m_dbkv->listObjects("profile_");
}

QVariantMap Core::getProfileInfo(const QString& profileId)
{
    if( !m_dbkv ) {
        qCWarning(Cc) << "No DBKV plugin available";
        return QVariantMap();
    }

    if( !m_dbsql ) {
        qCWarning(Cc) << "No DBSQL plugin available";
        return QVariantMap();
    }

    // TODO: Move to DBSQL
    QVariantMap object;
    if( !m_dbkv->retrieveObject("profile_" + profileId, object) ) {
        qCWarning(Cc) << "Failed to get profile:" << profileId;
        return QVariantMap();
    }

    return object;
}

void Core::setApp(QCoreApplication* app)
{
    m_app = app;
}

void Core::exit()
{
    m_app->exit(0);
}
