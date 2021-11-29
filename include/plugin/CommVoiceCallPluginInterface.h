#ifndef COMMVOICECALLPLUGININTERFACE_H
#define COMMVOICECALLPLUGININTERFACE_H

// Voice communication interface to make the phone calls

#include <QStringList>
#include "PluginInterface.h"

#define CommVoiceCallPluginInterface_iid "io.stateoftheart.asocial.plugin.CommVoiceCallPluginInterface"

class CommVoiceCallPluginInterface : virtual public CommPluginInterface
{
public:
    static QLatin1String type() { return QLatin1String(CommVoiceCallPluginInterface_iid); }
};

Q_DECLARE_INTERFACE(CommVoiceCallPluginInterface, CommVoiceCallPluginInterface_iid)

#endif // COMMVOICECALLPLUGININTERFACE_H
