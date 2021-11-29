#ifndef COMMVIDEOPLUGININTERFACE_H
#define COMMVIDEOPLUGININTERFACE_H

#include <QStringList>
#include "PluginInterface.h"

#define CommVideoPluginInterface_iid "io.stateoftheart.asocial.plugin.CommVideoPluginInterface"

class CommVideoPluginInterface : virtual public CommPluginInterface
{
public:
    static QLatin1String type() { return QLatin1String(CommVideoPluginInterface_iid); }
};

Q_DECLARE_INTERFACE(CommVideoPluginInterface, CommVideoPluginInterface_iid)

#endif // COMMVIDEOPLUGININTERFACE_H
