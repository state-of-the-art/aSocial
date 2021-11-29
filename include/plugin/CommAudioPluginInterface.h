#ifndef COMMAUDIOPLUGININTERFACE_H
#define COMMAUDIOPLUGININTERFACE_H

// Audio communication interface to send and receive audio files

#include <QStringList>
#include "PluginInterface.h"

#define CommAudioPluginInterface_iid "io.stateoftheart.asocial.plugin.CommAudioPluginInterface"

class CommAudioPluginInterface : virtual public CommPluginInterface
{
public:
    static QLatin1String type() { return QLatin1String(CommAudioPluginInterface_iid); }
};

Q_DECLARE_INTERFACE(CommAudioPluginInterface, CommAudioPluginInterface_iid)

#endif // COMMAUDIOPLUGININTERFACE_H
