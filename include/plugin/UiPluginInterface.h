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

#ifndef UIPLUGININTERFACE_H
#define UIPLUGININTERFACE_H

// Interface to implement the way application will display the content

#include "LogLevel.h"
#include "PluginInterface.h"

#include <QDebug>
#include <QStringList>

#define UiPluginInterface_iid "io.stateoftheart.asocial.plugin.UiPluginInterface"

/**
 * @brief Interface for the active UI plugin that also acts as the log sink.
 *
 * All log output from core and plugins is routed to the active UI plugin
 * so that display is controlled (e.g. ui-console can write through the
 * terminal without breaking the raw prompt). Default implementations
 * (e.g. ui-gui, ui-cmd) should use Qt's qDebug/qInfo/qWarning/qCritical/qFatal.
 */
class UiPluginInterface : virtual public PluginInterface
{
public:
    static QLatin1String type() { return QLatin1String(UiPluginInterface_iid); }

    virtual bool startUI() = 0;
    virtual bool stopUI() = 0;

    /** @brief Receives the log messages to print them out */
    virtual void logSink(LogLevel level, const QString& message)
    {
        switch( level ) {
        case LogLevel::Debug:
            qDebug().noquote() << message;
            break;
        case LogLevel::Info:
            qInfo().noquote() << message;
            break;
        case LogLevel::Warning:
            qWarning().noquote() << message;
            break;
        case LogLevel::Critical:
            qCritical().noquote() << message;
            break;
        case LogLevel::Fatal:
            qFatal("%s", qPrintable(message));
            break;
        }
    }
};

Q_DECLARE_INTERFACE(UiPluginInterface, UiPluginInterface_iid)

#endif // UIPLUGININTERFACE_H
