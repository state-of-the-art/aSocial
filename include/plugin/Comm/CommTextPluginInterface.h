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
