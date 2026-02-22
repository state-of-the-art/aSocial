#ifndef DBSQLPLUGININTERFACE_H
#define DBSQLPLUGININTERFACE_H

#include "PluginInterface.h"

#include <QVariantMap>
#include <QStringList>

#define DBSQLPluginInterface_iid "io.stateoftheart.asocial.plugin.DBSQLPluginInterface"

/**
 * Interface for database plugins that handle encrypted SQL storage
 */
class DBSQLPluginInterface : public PluginInterface
{
public:
    virtual ~DBSQLPluginInterface() {};

    /**
     * @brief Return plugin type
     */
    static QLatin1String type() { return QLatin1String(DBSQLPluginInterface_iid); }
};

Q_DECLARE_INTERFACE(DBSQLPluginInterface, DBSQLPluginInterface_iid)

#endif // DBSQLPLUGININTERFACE_H
