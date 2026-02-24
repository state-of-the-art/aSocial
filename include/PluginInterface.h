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

#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QtPlugin>
#include <QStringList>
#include "CoreInterface.h"

#define PluginInterface_iid "io.stateoftheart.asocial.PluginInterface"

/**
 * Basic interface for plugins
 */
class PluginInterface
{
public:
    virtual ~PluginInterface() {};

    /**
     * @brief Return plugin type
     */
    static QLatin1String type() { return QLatin1String(PluginInterface_iid); }

    /**
     * @brief Plugin identify name
     */
    Q_INVOKABLE virtual QString name() const = 0;

    /**
     * @brief List of the plugins required by the plugin
     */
    virtual QStringList requirements() const = 0;

    /**
     * @brief Executed during plugin activation
     * Warning: will be executed for each interface
     */
    virtual bool init() = 0;

    /**
     * @brief Passes the Core object to interact with the platform
     * Called during plugin initialization
     * @param core - ref to the Core object
     */
    void setCore(CoreInterface* core) { m_core = core; };

    /**
     * @brief Executed during plugin deactivation
     */
    virtual bool deinit() = 0;

    /**
     * @brief Executed when all the available plugins are initialized
     */
    virtual bool configure() = 0;

    /**
     * @brief Shows the plugin was initialized or not
     */
    bool isInitialized() { return m_initialized; }

signals:
    virtual void appNotice(QString msg) = 0;
    virtual void appWarning(QString msg) = 0;
    virtual void appError(QString msg) = 0;

protected:
    /**
     * @brief Used by plugin to set the init state
     * @param value
     */
    void setInitialized(bool value) { m_initialized = value; }

    CoreInterface* m_core;

private:
    bool m_initialized = false;
};

Q_DECLARE_INTERFACE(PluginInterface, PluginInterface_iid)

#endif // PLUGININTERFACE_H
