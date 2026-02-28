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

#ifndef UIPLUGININTERFACE_H
#define UIPLUGININTERFACE_H

// Interface to implement the way application will display the content

#include "PluginInterface.h"
#include <QStringList>

#define UiPluginInterface_iid "io.stateoftheart.asocial.plugin.UiPluginInterface"

class UiPluginInterface : virtual public PluginInterface
{
public:
    static QLatin1String type() { return QLatin1String(UiPluginInterface_iid); }

    virtual bool startUI() = 0;
    virtual bool stopUI() = 0;
};

Q_DECLARE_INTERFACE(UiPluginInterface, UiPluginInterface_iid)

#endif // UIPLUGININTERFACE_H
