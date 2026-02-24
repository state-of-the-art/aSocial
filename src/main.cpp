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

//#include <QtGui/QGuiApplication>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QLocale>
#include <QTranslator>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(Cm, "main")

#include "plugins.h"
#include "settings.h"
#include "core.h"

#include "plugin/UiPluginInterface.h"

int main(int argc, char *argv[])
{
    qCDebug(Cm, "Init v%s", PROJECT_VERSION);

    QScopedPointer<QCoreApplication> app(new QCoreApplication(argc, argv));
    QCoreApplication::setOrganizationName("State-Of-The-Art");
    QCoreApplication::setOrganizationDomain("state-of-the-art.io");
    QCoreApplication::setApplicationName("aSocial");
    QCoreApplication::setApplicationVersion(PROJECT_VERSION);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({
        {"no-gui", QCoreApplication::translate("main", "Load CMD instead of GUI in case GUI is available")},
    });
    parser.process(*app.data());

    // Loading translations
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for( const QString &locale : uiLanguages ) {
        const QString baseName = "asocial_" + QLocale(locale).name();
        if( translator.load(":/i18n/" + baseName) ) {
            app->installTranslator(&translator);
            break;
        }
    }

    qCInfo(Cm, "Init plugins...");
    Plugins::I();

    qCInfo(Cm, "Init settings...");
    Settings::I();

    qCInfo(Cm, "Init core...");
    Core::I()->setDBKVPlugin("dbkv-json");
    Core::I()->setDBSQLPlugin("dbsql-sqlite");

    if( parser.isSet("no-gui") ) {
        // Init console application
        qCDebug(Cm, "aSocial - console mode");
    } else {
        // Init GUI application
        qCDebug(Cm, "aSocial - GUI mode");
    }

    // Test cli ui
    Settings::I()->setting("plugins.ui.cmd.active", true);
    Plugins::I()->settingActivePlugin("plugins.ui.cmd.active", "ui-cmd");
    Plugins::I()->activateInterface("ui-cmd", QLatin1String("io.stateoftheart.asocial.plugin.UiPluginInterface"));
    UiPluginInterface* plugin = qobject_cast<UiPluginInterface *>(Plugins::I()->getPlugin("io.stateoftheart.asocial.plugin.UiPluginInterface", "ui-cmd"));
    if( plugin == nullptr ) {
        qCFatal(Cm, "Unable to locate UI plugin");
        return 1;
    }
    plugin->startUI();

    qCDebug(Cm, "UI Plugin activated");

    return app->exec();
}
