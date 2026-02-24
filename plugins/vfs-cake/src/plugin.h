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

#ifndef VFSCAKEPLUGIN_H
#define VFSCAKEPLUGIN_H

#include <QObject>
#include "plugin/VFSPluginInterface.h"

class Plugin : public QObject, public VFSPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "io.stateoftheart.asocial.plugin.VFSCakePlugin")
    Q_INTERFACES(VFSPluginInterface PluginInterface)

public:
    Plugin();
    ~Plugin() override;

    static Plugin* s_pInstance;

    // PluginInterface
    Q_INVOKABLE QString name() const override;
    QStringList requirements() const override;
    bool init() override;
    bool deinit() override;
    bool configure() override;

    // VFSPluginInterface
    VFSContainerPluginInterface* openContainer(
        const QString& containerPath,
        const QString& passphrase,
        quint64 maxContainerSize = 0) override;

signals:
    void appNotice(QString msg) override;
    void appWarning(QString msg) override;
    void appError(QString msg) override;
};

#endif // VFSCAKEPLUGIN_H
