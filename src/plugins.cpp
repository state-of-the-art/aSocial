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

#include "plugins.h"
#include "CoreAccessProxy.h"
#include "settings.h"

#include "Log.h"
#include <QCoreApplication>
#include <QDir>

#include <QPluginLoader>

#include "PluginInterface.h"
#include "core.h"
#include "plugin/CommPluginInterface.h"
#include "plugin/DBKVPluginInterface.h"
#include "plugin/UiPluginInterface.h"
#include "plugin/VFSPluginInterface.h"

Plugins* Plugins::s_pInstance = nullptr;

Plugins::Plugins()
    : QObject(nullptr)
{
    LOG_D() << "Create object";
    refreshPluginsList();
}

Plugins::~Plugins()
{
    LOG_D() << "Destroy object";
}

QList<QString> Plugins::listPlugins()
{
    return m_plugins.keys();
}

QList<QLatin1String> Plugins::listInterfaces(const QString& name)
{
    if( !m_plugins.contains(name) ) {
        LOG_W() << __func__ << "Not found plugin" << name;
        return QList<QLatin1String>();
    }
    return m_plugins[name].keys();
}

QList<QObject*> Plugins::getInterfacePlugins(const QString& if_name)
{
    QLatin1String iid = QLatin1String(if_name.toLatin1());
    if( m_plugins_active.contains(iid) )
        return m_plugins_active[iid];
    else {
        LOG_W() << "There is no plugins for" << if_name;
        return QList<QObject*>();
    }
}

QObject* Plugins::getPlugin(const QString& if_name, const QString& name)
{
    QLatin1String iid = QLatin1String(if_name.toLatin1());
    if( m_plugins_active.contains(iid) ) {
        for( QObject* plugin : m_plugins_active[iid] ) {
            if( qobject_cast<PluginInterface*>(plugin)->name() == name )
                return plugin;
        }
    }

    LOG_W() << __func__ << "There is no plugin found:" << if_name << name;
    return nullptr;
}

void Plugins::settingActivePlugin(const QString& key, const QString& name)
{
    m_setting_plugin_active.insert(key, name);
}

void Plugins::settingActiveInterface(const QString& key, const QString& name, const QLatin1String& interface)
{
    m_setting_plugin_interface_active.insert(key, QPair<QString, QLatin1String>(name, interface));
}

bool Plugins::activateInterface(const QString& name, const QLatin1String& interface_id)
{
    if( !Settings::I()->setting(m_setting_plugin_active.key(name)).toBool() )
        return false;
    QObject* plugin = m_plugins[name][interface_id];
    PluginInterface* plugin_if = qobject_cast<PluginInterface*>(plugin);
    if( !plugin_if ) {
        LOG_W() << __func__ << "Unable to locate plugin interface to activate" << name << plugin_if << plugin;
        return false;
    }

    // Create a per-plugin CoreAccessProxy if one does not yet exist
    if( !m_proxies.contains(name) ) {
        PluginPermissions perms = plugin_if->requiredPermissions();
        LOG_I() << "Granting permissions to" << name << "v" + plugin_if->version() << ":"
                << permissionNames(perms).join(", ");
        auto* proxy = new CoreAccessProxy(Core::I(), perms, name, this);
        m_proxies.insert(name, proxy);
    }

    // TODO: Use settings or LOGGER verbosity to specify verbosity of the plugins
    plugin_if->setLog(name, LogLevel::Debug, LOGGER);
    plugin_if->setCore(m_proxies.value(name));
    plugin_if->init();
    if( !m_plugins_active[interface_id].contains(plugin) ) {
        m_plugins_active[interface_id].append(plugin);
        m_activation_order.append(qMakePair(name, interface_id));
        return true;
    }
    return false;
}

bool Plugins::deactivateInterface(const QString& name, const QLatin1String& interface_id)
{
    LOG_D() << "Deactivating interface for" << name;
    QObject* plugin = m_plugins[name][interface_id];
    PluginInterface* plugin_if = qobject_cast<PluginInterface*>(plugin);
    if( !plugin_if ) {
        LOG_W() << __func__ << "Unable to locate plugin interface to deactivate" << name << plugin_if << plugin;
        return false;
    }
    if( m_plugins_active[interface_id].contains(plugin) ) {
        m_plugins_active[interface_id].removeOne(plugin);
        // Remove one matching entry from activation order (last match for LIFO)
        for( int i = m_activation_order.size() - 1; i >= 0; --i ) {
            if( m_activation_order[i].first == name && m_activation_order[i].second == interface_id ) {
                m_activation_order.removeAt(i);
                break;
            }
        }
        // Deinit plugin if no interface active anymore
        auto it = m_plugins_active.begin();
        while( it != m_plugins_active.end() ) {
            if( it.value().contains(plugin) )
                return true;
            ++it;
        }
        plugin_if->deinit();

        // Destroy the permission proxy once all interfaces are deactivated
        if( m_proxies.contains(name) ) {
            delete m_proxies.take(name);
            LOG_D() << "Destroyed CoreAccessProxy for" << name;
        }
        return true;
    }
    return false;
}

void Plugins::shutdownAllPluginsInReverseOrder()
{
    // Deactivate in reverse order so UI and dependent plugins shut down first
    for( int i = m_activation_order.size() - 1; i >= 0; --i ) {
        const QString name = m_activation_order[i].first;
        const QLatin1String interface_id = m_activation_order[i].second;
        deactivateInterface(name, interface_id);
    }
}

void Plugins::settingChanged(const QString& key, const QVariant& value)
{
    if( m_setting_plugin_active.contains(key) ) {
        QString name = m_setting_plugin_active[key];
        LOG_D() << "Set plugin" << name << "active" << value.toBool();
        if( value.toBool() ) {
            auto it = m_setting_plugin_interface_active.begin();
            while( it != m_setting_plugin_interface_active.end() ) {
                if( it.value().first == name && Settings::I()->setting(it.key()).toBool() )
                    activateInterface(name, it.value().second);
                ++it;
            }
        } else {
            auto it = m_setting_plugin_interface_active.begin();
            while( it != m_setting_plugin_interface_active.end() ) {
                if( it.value().first == name && Settings::I()->setting(it.key()).toBool() )
                    deactivateInterface(name, it.value().second);
                ++it;
            }
        }
    } else if( m_setting_plugin_interface_active.contains(key) ) {
        auto info = m_setting_plugin_interface_active[key];
        LOG_D() << "Set plugin" << info.first << "interface" << info.second << "active" << value.toBool();
        if( value.toBool() )
            activateInterface(info.first, info.second);
        else
            deactivateInterface(info.first, info.second);
    }
}

void Plugins::refreshPluginsList()
{
    // By default uses simple structure where plugins located near the executable
    QStringList plugins_dirs = {QCoreApplication::applicationDirPath().append("/plugins")};
    plugins_dirs.append(QCoreApplication::applicationDirPath().append("/../share/asocial/plugins"));
    // TODO: Could be dangerous to load plugins from everywhere - maybe by default load
    // only the trusted ones or implement plugin vendors and signatures?
    plugins_dirs.append(Settings::I()->setting("workdir.applocaldata").toString());

    QStringList filters = {"libasocial-plugin-*"};

    for( const QString& path : plugins_dirs ) {
        QDir dir = QDir(path);
        if( !dir.exists() ) {
            continue;
        }
        LOG_D() << "Listing plugins from directory:" << path;
        dir.setNameFilters(filters);
        QStringList libs = dir.entryList(QDir::Files);
        for( const QString& lib_name : libs ) {
            QPluginLoader plugin_loader(dir.absoluteFilePath(lib_name), this);
            QObject* plugin = plugin_loader.instance();
            if( !plugin ) {
                LOG_W() << "  unable to load plugin:" << lib_name << plugin_loader.errorString();
                continue;
            }

            // Set parent of plugin to make sure it will not be destroyed accidentally
            plugin->setParent(this);

            // Connecting the message signals
            /*connect(plugin, SIGNAL(appNotice(QString)), Application::I(), SLOT(notice(QString)));
            connect(plugin, SIGNAL(appWarning(QString)), Application::I(), SLOT(warning(QString)));
            connect(plugin, SIGNAL(appError(QString)), Application::I(), SLOT(error(QString)));*/

            PluginInterface* base_if = qobject_cast<PluginInterface*>(plugin);
            if( base_if ) {
                LOG_I() << "  loading plugin:" << base_if->name() << "v" + base_if->version() << "from" << lib_name;
                LOG_D() << "    requested permissions:" << permissionNames(base_if->requiredPermissions()).join(", ");
            } else {
                LOG_D() << "  loading plugin:" << lib_name;
            }

            bool loaded = false;
            loaded = addPlugin<UiPluginInterface>(qobject_cast<UiPluginInterface*>(plugin), plugin);
            loaded = addPlugin<CommPluginInterface>(qobject_cast<CommPluginInterface*>(plugin), plugin) || loaded;
            loaded = addPlugin<VFSPluginInterface>(qobject_cast<VFSPluginInterface*>(plugin), plugin) || loaded;
            loaded = addPlugin<DBKVPluginInterface>(qobject_cast<DBKVPluginInterface*>(plugin), plugin) || loaded;

            if( !loaded ) {
                plugin_loader.unload();
                LOG_W() << "  no supported interfaces found for plugin:" << lib_name;
            }
        }
    }

    auto it = m_plugins.begin();
    while( it != m_plugins.end() ) {
        auto it2 = it.value().begin();
        while( it2 != it.value().end() ) {
            LOG_D() << "Loaded plugin" << it.key() << "for interface" << it2.key() << ":" << it2.value();
            ++it2;
        }
        ++it;
    }
}

template<class T>
bool Plugins::addPlugin(T* obj, QObject* plugin)
{
    static_assert(std::is_base_of<PluginInterface, T>::value, "Unable to add non-PluginInterface object as plugin");

    if( !obj )
        return false;

    if( m_plugins.contains(obj->name()) && m_plugins[obj->name()].contains(obj->type()) ) {
        LOG_W() << "  plugin already loaded, skipping:" << obj->name() << "::" << obj->type();
        return false;
    }

    m_plugins[obj->name()][obj->type()] = plugin;
    LOG_D() << "  loaded plugin:" << obj->name() << "::" << obj->type();
    return true;
}
