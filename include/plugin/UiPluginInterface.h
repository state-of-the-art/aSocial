#ifndef UIPLUGININTERFACE_H
#define UIPLUGININTERFACE_H

// Interface to implement the way application will display the content

#include <QStringList>
#include "PluginInterface.h"

#define UiPluginInterface_iid "io.stateoftheart.asocial.plugin.UiPluginInterface"

class UiPluginInterface : virtual public PluginInterface
{
public:
    static QLatin1String type() { return QLatin1String(UiPluginInterface_iid); }
};

Q_DECLARE_INTERFACE(UiPluginInterface, UiPluginInterface_iid)

#endif // UIPLUGININTERFACE_H
