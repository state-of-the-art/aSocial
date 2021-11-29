#ifndef COMMDATAPLUGININTERFACE_H
#define COMMDATAPLUGININTERFACE_H

// Data communication interface to transfer binaries and files

#include <QStringList>
#include "PluginInterface.h"

#define CommDataPluginInterface_iid "io.stateoftheart.asocial.plugin.CommDataPluginInterface"

class CommDataPluginInterface : virtual public CommPluginInterface
{
public:
    static QLatin1String type() { return QLatin1String(CommDataPluginInterface_iid); }
};

Q_DECLARE_INTERFACE(CommDataPluginInterface, CommDataPluginInterface_iid)

#endif // COMMDATAPLUGININTERFACE_H
