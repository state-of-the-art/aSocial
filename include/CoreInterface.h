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

#ifndef COREINTERFACE_H
#define COREINTERFACE_H

#include <QObject>
#include <QStringList>
#include <QVariantMap>

class DBKVPluginInterface;
class DBSQLPluginInterface;

/**
 * Abstract interface for Core functionality that plugins can use
 * This ensures binary compatibility when Core is extended
 */
class CoreInterface
{
public:
    virtual ~CoreInterface() {}

    // Key-Value database management
    virtual DBKVPluginInterface* getDBKV() const = 0;
    // SQL database management
    virtual DBSQLPluginInterface* getDBSQL() const = 0;

    // Profile management
    virtual QString getCurrentProfileId() const = 0;
    virtual void setCurrentProfileId(const QString& profileId) = 0;

    // Profile operations
    virtual QString createProfile(const QString& name) = 0;
    virtual QString importProfile(const QString& serializedData) = 0;
    virtual QString exportProfile(const QString& profileId) = 0;
    virtual bool deleteProfile(const QString& profileId) = 0;
    virtual QStringList listProfiles() = 0;
    virtual QVariantMap getProfileInfo(const QString& profileId) = 0;

    // Core app features
    virtual void exit() = 0;

signals:
    virtual void currentProfileChanged(const QString& profileId) = 0;
};

Q_DECLARE_INTERFACE(CoreInterface, "io.stateoftheart.asocial.CoreInterface")

#endif // COREINTERFACE_H
