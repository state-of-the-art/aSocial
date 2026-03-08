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

#include "CoreInterface.h"
#include "PluginPermissions.h"
#include <QStringList>
#include <QtPlugin>

#define PluginInterface_iid "io.stateoftheart.asocial.PluginInterface"

/**
 * @brief Base interface that every aSocial plugin must implement.
 *
 * Provides identity (name, version), lifecycle hooks (init / deinit /
 * configure), dependency declaration, and the granular permission system
 * that controls which Core capabilities a plugin may access.
 *
 * The platform calls setCore() with a CoreAccessProxy whose permission
 * mask matches the flags returned by requiredPermissions().  Any method
 * call outside the granted set is denied at runtime and logged.
 */
class PluginInterface
{
public:
    virtual ~PluginInterface(){};

    /**
     * @brief Return the Qt interface identifier for the base plugin type.
     * @return Compile-time IID string.
     */
    static QLatin1String type() { return QLatin1String(PluginInterface_iid); }

    /**
     * @brief Human-readable plugin name (e.g. "ui-cmd", "dbkv-rocksdb").
     *
     * Typically backed by the PLUGIN_NAME compile definition.
     */
    Q_INVOKABLE virtual QString name() const = 0;

    /**
     * @brief Semantic version string of this plugin build.
     *
     * Typically backed by the PLUGIN_VERSION compile definition set in
     * plugins/plugin.cmake from the plugin's project(VERSION ...) call.
     *
     * @return Version string such as "0.1.0".
     */
    virtual QString version() const = 0;

    /**
     * @brief Declare the Core capabilities this plugin requires.
     *
     * The platform inspects these flags before activation and constructs
     * a CoreAccessProxy that permits only the declared operations.
     * In the future a UI will let the user approve or deny each flag.
     *
     * @return Bitwise OR of PluginPermission flags.
     */
    virtual PluginPermissions requiredPermissions() const = 0;

    /**
     * @brief List of plugin names this plugin depends on.
     * @return Names that must be loaded before this plugin can initialise.
     */
    virtual QStringList requirements() const = 0;

    /**
     * @brief Executed during plugin activation.
     *
     * May be called once per interface the plugin implements.
     *
     * @return true on success.
     */
    virtual bool init() = 0;

    /**
     * @brief Inject the Core access proxy for this plugin.
     *
     * Called by the platform during activation.  The supplied pointer is
     * a permission-gated proxy — only operations matching the plugin's
     * requiredPermissions() are allowed.
     *
     * @param core  Permission-gated CoreInterface proxy (non-owning).
     */
    void setCore(CoreInterface* core) { m_core = core; };

    /**
     * @brief Executed during plugin deactivation.
     * @return true on success.
     */
    virtual bool deinit() = 0;

    /**
     * @brief Executed when all available plugins are initialized.
     * @return true on success.
     */
    virtual bool configure() = 0;

    /**
     * @brief Query whether this plugin has been initialised.
     * @return true if init() completed successfully.
     */
    bool isInitialized() { return m_initialized; }

signals:
    virtual void appNotice(QString msg) = 0;
    virtual void appWarning(QString msg) = 0;
    virtual void appError(QString msg) = 0;

protected:
    /**
     * @brief Set the initialisation flag (called from init/deinit).
     * @param value  New init state.
     */
    void setInitialized(bool value) { m_initialized = value; }

    /** @brief Permission-gated proxy to Core functionality. */
    CoreInterface* m_core = nullptr;

private:
    bool m_initialized = false;
};

Q_DECLARE_INTERFACE(PluginInterface, PluginInterface_iid)

#endif // PLUGININTERFACE_H
