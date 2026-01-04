#ifndef COMMTEXTPLUGININTERFACE_H
#define COMMTEXTPLUGININTERFACE_H

// Simple text communication interface

#include <QStringList>
#include "../CommPluginInterface.h"

#define CommTextPluginInterface_iid "io.stateoftheart.asocial.plugin.CommTextPluginInterface"

class CommTextPluginInterface : virtual public CommPluginInterface
{
public:
    static QLatin1String type() { return QLatin1String(CommTextPluginInterface_iid); }
};

Q_DECLARE_INTERFACE(CommTextPluginInterface, CommTextPluginInterface_iid)

#endif // COMMTEXTPLUGININTERFACE_H
