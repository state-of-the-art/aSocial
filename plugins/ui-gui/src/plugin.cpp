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
#include "GuiBackend.h"
#include "Log.h"

#include <QCoreApplication>
#include <QDir>
#include <QQmlApplicationEngine>
#include <QQmlContext>

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
    return QStringList();
}

bool Plugin::init()
{
    if( isInitialized() )
        return true;

    LOG_D() << __func__;
    Plugin::s_pInstance = this;

    m_backend = new GuiBackend(this);

    LOG_D() << "init() done";
    setInitialized(true);
    emit appNotice(this->name().append(" initialized"));
    return true;
}

bool Plugin::deinit()
{
    if( !isInitialized() )
        return true;
    LOG_D() << __func__;

    stopUI();

    delete m_backend;
    m_backend = nullptr;

    Plugin::s_pInstance = nullptr;
    emit appNotice(this->name().append(" deinitialized"));
    LOG_D() << "deinit() done";
    setInitialized(false);
    return true;
}

bool Plugin::configure()
{
    LOG_D() << __func__;
    return true;
}

bool Plugin::startUI()
{
    LOG_D() << __func__;

    if( !m_backend || !m_core ) {
        LOG_W() << "Cannot start UI: backend or core not available";
        return false;
    }

    m_backend->setCore(m_core);

    m_engine = new QQmlApplicationEngine(this);

    m_engine->rootContext()->setContextProperty(QStringLiteral("Backend"), m_backend);

    // Resolve QML path: try plugin-relative first, then appdir-relative
    QString qmlDir;
    QString appDir = QCoreApplication::applicationDirPath();

    // In AppImage, QML lives alongside the binary in qml/
    QStringList searchPaths = {
        //appDir + QStringLiteral("/qml/ui-gui"),
        //appDir + QStringLiteral("/../share/asocial/qml/ui-gui"),
        QStringLiteral(":/asocial/plugin/ui-gui/qml"),
    };

    for( const QString& path : searchPaths ) {
        if( QDir(path).exists(QStringLiteral("Main.qml")) ) {
            qmlDir = path;
            break;
        }
    }

    if( qmlDir.isEmpty() ) {
        // Fallback: load from embedded QRC
        LOG_C() << "Failed to find Main.qml in provided search paths";
        delete m_engine;
        m_engine = nullptr;
        return false;
    }

    LOG_D() << "Loading QML from:" << qmlDir;
    m_engine->load(QUrl::fromLocalFile(qmlDir + QStringLiteral("/Main.qml")));

    if( m_engine->rootObjects().isEmpty() ) {
        LOG_W() << "Failed to load QML UI";
        delete m_engine;
        m_engine = nullptr;
        return false;
    }

    LOG_D() << "Qt Quick UI started successfully";
    return true;
}

bool Plugin::stopUI()
{
    LOG_D() << __func__;

    if( m_engine ) {
        delete m_engine;
        m_engine = nullptr;

        // Destroying backend after QML engine is stopped
        delete m_backend;
        m_backend = nullptr;
    }

    return true;
}
