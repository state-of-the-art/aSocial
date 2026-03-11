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

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QLocale>
#include <QStandardPaths>
#include <QTranslator>

#include "Log.h"

#include "core.h"
#include "plugins.h"
#include "settings.h"

#include "plugin/UiPluginInterface.h"

int main(int argc, char* argv[])
{
    LOGGER = new Log("aSocial", LogLevel::Debug, nullptr);
    LOG_D() << "Init v" << PROJECT_VERSION;

    QScopedPointer<QCoreApplication> app(new QCoreApplication(argc, argv));
    QCoreApplication::setOrganizationName("State-Of-The-Art");
    QCoreApplication::setOrganizationDomain("state-of-the-art.io");
    QCoreApplication::setApplicationName("aSocial");
    QCoreApplication::setApplicationVersion(PROJECT_VERSION);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption
        noUiOption("no-ui", QCoreApplication::translate("main", "Load CMD instead of GUI in case GUI is available"));
    parser.addOption(noUiOption);
    QCommandLineOption workingDirectoryOption(
        QStringList() << "w",
        "workdir",
        QCoreApplication::translate("main", "Set working directory instead of default one"));
    parser.addOption(workingDirectoryOption);

    parser.process(*app.data());

    // Loading translations
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for( const QString& locale : uiLanguages ) {
        const QString baseName = "asocial_" + QLocale(locale).name();
        if( translator.load(":/i18n/" + baseName) ) {
            app->installTranslator(&translator);
            break;
        }
    }

    LOG_I() << "Init settings...";
    if( parser.isSet(workingDirectoryOption) ) {
        const QString wd = parser.value(workingDirectoryOption);
        QString conf_path = QDir(wd).absolutePath() + QDir::separator() + "settings.ini";
        Settings::setConfigFile(conf_path);
        Settings::I()->setDefault("workdir.appdata", wd);
        Settings::I()->setDefault("workdir.applocaldata", wd);
    } else {
        Settings::I();
        Settings::I()->setDefault("workdir.appdata", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
        Settings::I()
            ->setDefault("workdir.applocaldata", QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    }

    Settings::I()->setDefault(
        "vfs.container.path", Settings::I()->setting("workdir.appdata").toString() + QDir::separator() + "data.vfs");

    LOG_I() << "Init plugins...";
    Plugins::I();

    // Seed default settings for plugin selection
    Settings::I()->setDefault("vfs.plugin", "vfs-cake");
    Settings::I()->setDefault("dbkv.plugin", "dbkv-json");
    Settings::I()->setDefault("dbkv.profile.plugin", "dbkv-rocksdb");

    LOG_I() << "Init core...";
    Core::I()->setApp(app.data());
    Core::I()->setDBKVPlugin(Settings::I()->setting("dbkv.plugin").toString());
    Core::I()->setDBKVProfilePlugin(Settings::I()->setting("dbkv.profile.plugin").toString());
    Core::I()->setVFSPlugin(Settings::I()->setting("vfs.plugin").toString());
    Core::I()->init();

    if( parser.isSet(noUiOption) ) {
        LOG_D() << "aSocial - console mode";
    } else {
        LOG_D() << "aSocial - UI mode";
    }

    // Activate UI plugin
    Settings::I()->setting("plugins.ui.console.active", true);
    Plugins::I()->settingActivePlugin("plugins.ui.console.active", "ui-console");
    Plugins::I()->activateInterface("ui-console", QLatin1String("io.stateoftheart.asocial.plugin.UiPluginInterface"));
    UiPluginInterface* plugin = qobject_cast<UiPluginInterface*>(
        Plugins::I()->getPlugin("io.stateoftheart.asocial.plugin.UiPluginInterface", "ui-console"));
    if( plugin == nullptr ) {
        LOG_F() << "Unable to locate UI plugin";
        return 1;
    }
    LOGGER->setSink(plugin);
    plugin->startUI();

    LOG_D() << "UI Plugin activated";

    return app->exec();
}
