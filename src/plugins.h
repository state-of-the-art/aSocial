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

    QList<QString> listPlugins();
    QList<QLatin1String> listInterfaces(const QString& name);

    // Plugin functions
    Q_INVOKABLE QList<QObject*> getInterfacePlugins(const QString& name);
    Q_INVOKABLE QObject* getPlugin(const QString& if_name, const QString& name);

    // Settings binding functions
    void settingActivePlugin(const QString& key, const QString& name);
    void settingActiveInterface(const QString& key, const QString& name, const QLatin1String& interface_id);

    bool activateInterface(const QString& name, const QLatin1String& interface_id);
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

    QMap<QString, QString> m_setting_plugin_active; // setting_key : plugin_name
    QMap<QString, QPair<QString, QLatin1String>>
        m_setting_plugin_interface_active; // setting_key : (plugin_name, interface_id)
};

#endif // PLUGINS_H
