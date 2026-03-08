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

#include "uiworker.h"

Q_LOGGING_CATEGORY(C, PLUGIN_NAME)

Plugin* Plugin::s_pInstance = nullptr;

QString Plugin::name() const
{
    return QLatin1String(PLUGIN_NAME);
}

QString Plugin::version() const
{
    return QStringLiteral(PLUGIN_VERSION);
}

PluginPermissions Plugin::requiredPermissions() const
{
    // The CLI exposes all Core functionality to the user
    return PluginPermission::SettingsRead | PluginPermission::SettingsWrite | PluginPermission::ContainerRead
           | PluginPermission::ContainerWrite | PluginPermission::ProfileRead | PluginPermission::ProfileWrite
           | PluginPermission::ProfileDelete | PluginPermission::ProfileExport | PluginPermission::ProfileImport
           | PluginPermission::DataRead | PluginPermission::DataWrite | PluginPermission::AppLifecycle;
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

    ui_thread = new UiWorker();
    ui_thread->setCore(m_core);

    configure();

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

    stopUI();

    delete ui_thread;

    Plugin::s_pInstance = nullptr;

    emit appNotice(this->name().append(" deinitialized"));
    qCDebug(C) << "deinit() done";
    setInitialized(false);

    return true;
}

bool Plugin::configure()
{
    qCDebug(C) << __func__;

    ui_thread->configure();

    return true;
}

bool Plugin::startUI()
{
    qCDebug(C) << __func__;

    ui_thread->start();

    return true;
}

bool Plugin::stopUI()
{
    qCDebug(C) << __func__;

    ui_thread->quit();
    ui_thread->wait();

    return true;
}
