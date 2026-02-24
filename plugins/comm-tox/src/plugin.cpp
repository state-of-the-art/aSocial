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

#include "plugin.h"

#include <QLoggingCategory>
#include "tox/tox.h"

Q_LOGGING_CATEGORY(C, PLUGIN_NAME)

Plugin* Plugin::s_pInstance = nullptr;

QString Plugin::name() const
{
    return QLatin1String(PLUGIN_NAME);
}

QStringList Plugin::requirements() const
{
    qCDebug(C) << __func__;
    return QStringList();
}

bool Plugin::init()
{
    if( isInitialized() )
        return true;

    qCDebug(C) << __func__;
    Plugin::s_pInstance = this;

    qCDebug(C) << "Tox version:" << tox_version_major() << tox_version_minor() << tox_version_patch();
    // TODO: Init here

    qCDebug(C) << "init() done";

    setInitialized(true);

    emit appNotice(this->name().append(" initialized"));

    return true;
}

bool Plugin::deinit()
{
    if( !isInitialized() )
        return true;
    qCDebug(C) << __func__;

    // TODO: Deinit here

    Plugin::s_pInstance = nullptr;

    emit appNotice(this->name().append(" deinitialized"));
    qCDebug(C) << "deinit() done";
    setInitialized(false);

    return true;
}

bool Plugin::configure()
{
    qCDebug(C) << __func__;
    return true;
}
