// Copyright (C) 2026  aSocial Developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// Author: Rabit (@rabits)

#ifndef COMMAUDIOPLUGININTERFACE_H
#define COMMAUDIOPLUGININTERFACE_H

// Audio communication interface to send and receive audio files

#include <QStringList>
#include "../CommPluginInterface.h"

#define CommAudioPluginInterface_iid "io.stateoftheart.asocial.plugin.CommAudioPluginInterface"

class CommAudioPluginInterface : virtual public CommPluginInterface
{
public:
    static QLatin1String type() { return QLatin1String(CommAudioPluginInterface_iid); }
};

Q_DECLARE_INTERFACE(CommAudioPluginInterface, CommAudioPluginInterface_iid)

#endif // COMMAUDIOPLUGININTERFACE_H
