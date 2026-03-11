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
#include "ConsoleUI.h"
#include "Log.h"

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
    return PluginPermission::SettingsRead | PluginPermission::SettingsWrite | PluginPermission::ContainerRead
           | PluginPermission::ContainerWrite | PluginPermission::ProfileRead | PluginPermission::ProfileWrite
           | PluginPermission::ProfileDelete | PluginPermission::ProfileExport | PluginPermission::ProfileImport
           | PluginPermission::DataRead | PluginPermission::DataWrite | PluginPermission::AppLifecycle;
}

QStringList Plugin::requirements() const
{
    LOG_D() << __func__;
    return {};
}

bool Plugin::init()
{
    if( isInitialized() )
        return true;

    LOG_D() << __func__;
    Plugin::s_pInstance = this;

    m_worker = new ConsoleUI();
    m_worker->setCore(m_core);

    configure();

    LOG_D() << "init() done";

    setInitialized(true);

    emit appNotice(this->name().append(QStringLiteral(" initialized")));

    return true;
}

bool Plugin::deinit()
{
    if( !isInitialized() )
        return true;
    LOG_D() << __func__;

    stopUI();

    delete m_worker;
    m_worker = nullptr;

    Plugin::s_pInstance = nullptr;

    emit appNotice(this->name().append(QStringLiteral(" deinitialized")));
    LOG_D() << "deinit() done";
    setInitialized(false);

    return true;
}

bool Plugin::configure()
{
    LOG_D() << __func__;

    m_worker->configure();

    return true;
}

bool Plugin::startUI()
{
    LOG_D() << __func__;

    m_worker->start();

    return true;
}

bool Plugin::stopUI()
{
    LOG_D() << __func__;

    m_worker->quit();
    m_worker->wait();

    return true;
}

void Plugin::logSink(LogLevel level, const QString& message)
{
    if( m_worker )
        m_worker->enqueueLog(level, message);
}
