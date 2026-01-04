#ifndef DATABASEPLUGININTERFACE_H
#define DATABASEPLUGININTERFACE_H

#include "PluginInterface.h"

#include <QVariantMap>
#include <QStringList>

#define DatabasePluginInterface_iid "io.stateoftheart.asocial.plugin.DatabasePluginInterface"

/**
 * Interface for database plugins that handle account storage
 */
class DatabasePluginInterface : public PluginInterface
{
public:
    virtual ~DatabasePluginInterface() {};

    /**
     * @brief Return plugin type
     */
    static QLatin1String type() { return QLatin1String(DatabasePluginInterface_iid); }

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

Q_DECLARE_INTERFACE(DatabasePluginInterface, DatabasePluginInterface_iid)

#endif // DATABASEPLUGININTERFACE_H
