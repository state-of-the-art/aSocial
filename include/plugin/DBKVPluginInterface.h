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
#include <QVariantMap>

#define DBKVPluginInterface_iid "io.stateoftheart.asocial.plugin.DBKVPluginInterface"

/**
 * Interface for database plugins that handle unencrypted storage
 */
class DBKVPluginInterface : public PluginInterface
{
public:
    virtual ~DBKVPluginInterface(){};

    /**
     * @brief Return plugin type
     */
    static QLatin1String type() { return QLatin1String(DBKVPluginInterface_iid); }

    /**
     * @brief List available objects
     * @param prefix Object key prefix
     * @return QStringList with list of found objects
     */
    virtual QStringList listObjects(const QString& prefix) = 0;

    /**
     * @brief Store arbitrary object
     * @param key Object key (filename for JSON storage)
     * @param object QVariantMap to serialize and store
     * @return true on success, false on failure
     */
    virtual bool storeObject(const QString& key, QVariantMap& object) = 0;

    /**
     * @brief Retrieve arbitrary object
     * @param key Object key
     * @param object QVariantMap to deserialize into
     * @return true on success, false on failure
     */
    virtual bool retrieveObject(const QString& key, QVariantMap& object) = 0;

    /**
     * @brief Check if object exists
     * @param key Object key
     * @return true if exists, false otherwise
     */
    virtual bool objectExists(const QString& key) = 0;

    /**
     * @brief Delete object
     * @param key Object key
     * @return true on success, false on failure
     */
    virtual bool deleteObject(const QString& key) = 0;
};

Q_DECLARE_INTERFACE(DBKVPluginInterface, DBKVPluginInterface_iid)

#endif // DBKVPLUGININTERFACE_H
