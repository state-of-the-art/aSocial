#ifndef COMMPLUGININTERFACE_H
#define COMMPLUGININTERFACE_H

// General communication interface which allow to transfer data from one point to another

#include <QStringList>
#include "PluginInterface.h"

#define CommPluginInterface_iid "io.stateoftheart.asocial.plugin.CommPluginInterface"

class CommPluginInterface : virtual public PluginInterface
{
public:
    static QLatin1String type() { return QLatin1String(CommPluginInterface_iid); }
};

Q_DECLARE_INTERFACE(CommPluginInterface, CommPluginInterface_iid)

#endif // COMMPLUGININTERFACE_H
