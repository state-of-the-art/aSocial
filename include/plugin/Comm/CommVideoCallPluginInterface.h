#ifndef COMMVIDEOCALLPLUGININTERFACE_H
#define COMMVIDEOCALLPLUGININTERFACE_H

// Video communication interface to make the video calls

#include <QStringList>
#include "../CommPluginInterface.h"

#define CommVideoCallPluginInterface_iid "io.stateoftheart.asocial.plugin.CommVideoCallPluginInterface"

class CommVideoCallPluginInterface : virtual public CommPluginInterface
{
public:
    static QLatin1String type() { return QLatin1String(CommVideoCallPluginInterface_iid); }
};

Q_DECLARE_INTERFACE(CommVideoCallPluginInterface, CommVideoCallPluginInterface_iid)

#endif // COMMVIDEOCALLPLUGININTERFACE_H
