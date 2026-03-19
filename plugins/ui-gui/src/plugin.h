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

#ifndef PLUGIN_H
#define PLUGIN_H

#include "plugin/UiPluginInterface.h"
#include <QObject>

class QQmlApplicationEngine;
class GuiBackend;

/**
 * @brief Qt Quick GUI plugin for aSocial.
 *
 * Implements UiPluginInterface by spinning up a QQmlApplicationEngine
 * with the bundled QML UI.  The GuiBackend singleton bridges QML to
 * the CoreInterface CRUD operations.
 *
 * Touch-ready: designed for both desktop mouse and Android touch.
 */
class Plugin : public QObject, public UiPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "io.stateoftheart.asocial.plugin.UiGuiPlugin")
    Q_INTERFACES(UiPluginInterface PluginInterface)

public:
    Plugin() {}
    static Plugin* s_pInstance;
    ~Plugin() override {}

    // PluginInterface
    Q_INVOKABLE QString name() const override;
    QString version() const override;
    PluginPermissions requiredPermissions() const override;
    QStringList requirements() const override;
    bool init() override;
    bool deinit() override;
    bool configure() override;

    // UiPluginInterface
    bool startUI() override;
    bool stopUI() override;

signals:
    void appNotice(QString msg) override;
    void appWarning(QString msg) override;
    void appError(QString msg) override;

private:
    QQmlApplicationEngine* m_engine = nullptr;
    GuiBackend* m_backend = nullptr;
};
#endif // PLUGIN_H
