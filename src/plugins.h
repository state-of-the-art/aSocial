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

#ifndef PLUGINS_H
#define PLUGINS_H

#include <QMap>
#include <QObject>

class CoreAccessProxy;

/**
 * @brief Singleton that discovers, loads and manages aSocial plugins.
 *
 * During startup the plugin directories are scanned for shared libraries
 * matching the "libasocial-plugin-*" pattern.  Each library is probed for
 * known Qt plugin interfaces (UI, Comm, VFS, DBKV, DBSQL).
 *
 * On activation a per-plugin CoreAccessProxy is created whose permission
 * mask matches the flags returned by PluginInterface::requiredPermissions().
 * This proxy is the only access path to Core for the plugin.
 */
class Plugins : public QObject
{
    Q_OBJECT

public:
    inline static Plugins* I()
    {
        if( s_pInstance == nullptr )
            s_pInstance = new Plugins();
        return s_pInstance;
    }
    inline static void destroyI() { delete s_pInstance; }

    /** @brief List all discovered plugin names. */
    QList<QString> listPlugins();

    /** @brief List interface IIDs that a named plugin implements. */
    QList<QLatin1String> listInterfaces(const QString& name);

    /** @brief Return all active plugin instances for an interface IID. */
    Q_INVOKABLE QList<QObject*> getInterfacePlugins(const QString& name);

    /** @brief Locate a specific active plugin by interface IID and name. */
    Q_INVOKABLE QObject* getPlugin(const QString& if_name, const QString& name);

    /** @brief Bind a settings key to toggling a whole plugin on/off. */
    void settingActivePlugin(const QString& key, const QString& name);

    /** @brief Bind a settings key to toggling one interface of a plugin. */
    void settingActiveInterface(const QString& key, const QString& name, const QLatin1String& interface_id);

    /**
     * @brief Activate one interface of a plugin.
     *
     * Creates a CoreAccessProxy with the plugin's declared permissions
     * and injects it via PluginInterface::setCore().
     *
     * @return true if the interface was activated.
     */
    bool activateInterface(const QString& name, const QLatin1String& interface_id);

    /**
     * @brief Deactivate one interface of a plugin.
     *
     * When no interfaces remain active for this plugin instance its
     * CoreAccessProxy is destroyed and deinit() is called.
     *
     * @return true if the interface was deactivated.
     */
    bool deactivateInterface(const QString& name, const QLatin1String& interface_id);

public slots:
    void settingChanged(const QString& key, const QVariant& value);

private:
    static Plugins* s_pInstance;
    explicit Plugins();
    ~Plugins() override;

    void refreshPluginsList();
    template<class T>
    bool addPlugin(T* obj, QObject* plugin);

    QMap<QString, QMap<QLatin1String, QObject*>> m_plugins; // plugin_name : interface_id : plugin_instance
    QMap<QLatin1String, QList<QObject*>> m_plugins_active;  // interface_id : list of plugin_instance

    /** @brief Per-plugin CoreAccessProxy instances (keyed by plugin name). */
    QMap<QString, CoreAccessProxy*> m_proxies;

    QMap<QString, QString> m_setting_plugin_active; // setting_key : plugin_name
    QMap<QString, QPair<QString, QLatin1String>>
        m_setting_plugin_interface_active; // setting_key : (plugin_name, interface_id)
};

#endif // PLUGINS_H
