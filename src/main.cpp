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

#include <exception>
#include <QCommandLineParser>
#include <QDir>
#include <QLocale>
#include <QStandardPaths>
#include <QTranslator>
#ifdef ASOCIAL_CLI_ONLY
#include <QCoreApplication>
#else
#include <QGuiApplication>
#endif

#include "Log.h"

#include "core.h"
#include "plugins.h"
#include "settings.h"

#include "plugin/UiPluginInterface.h"

#if defined(Q_OS_UNIX) && !defined(Q_OS_ANDROID)
#include <csignal>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <QSocketNotifier>

static int s_sigintFd = -1;

static void sigintHandler(int)
{
    if( s_sigintFd >= 0 ) {
        char c = 1;
        write(s_sigintFd, &c, 1);
    }
}
#endif

/**
 * @brief Run graceful shutdown: close profile, stop UI, deactivate plugins, destroy singletons.
 *
 * Called from QCoreApplication::aboutToQuit and from the top-level exception
 * handler in main() so that both normal quit and fatal error paths tear down
 * in the same order and avoid use-after-free (e.g. GuiBackend).
 */
static void runShutdown()
{
    Log* log = LOGGER;
    if( log )
        log->setSink(nullptr);

    Plugins* plugins = Plugins::I();
    if( plugins )
        plugins->shutdownAllPluginsInReverseOrder();

    Core::I()->closeProfile();

    Core::destroyI();
    Plugins::destroyI();
}

QCoreApplication* createApplication(int& argc, char* argv[])
{
#ifdef ASOCIAL_CLI_ONLY
    return new QCoreApplication(argc, argv);
#else
    for( int i = 1; i < argc; ++i ) {
        if( qstrcmp(argv[i], "--no-gui") == 0 )
            return new QCoreApplication(argc, argv);
    }
    return new QGuiApplication(argc, argv);
#endif
}

int main(int argc, char* argv[])
{
    LOGGER = new Log("aSocial", LogLevel::Debug, nullptr);
    LOG_D() << "Init v" << PROJECT_VERSION;
    int exitCode = EXIT_SUCCESS;

    try {
        QScopedPointer<QCoreApplication> app(createApplication(argc, argv));
        QCoreApplication::setOrganizationName("State-Of-The-Art");
        QCoreApplication::setOrganizationDomain("state-of-the-art.io");
        QCoreApplication::setApplicationName("aSocial");
        QCoreApplication::setApplicationVersion(PROJECT_VERSION);

        QCommandLineParser parser;
        parser.addHelpOption();
        parser.addVersionOption();

        QCommandLineOption guiOption("gui", QCoreApplication::translate("main", "Load GUI interface no matter what"));
        parser.addOption(guiOption);
        QCommandLineOption noGuiOption(
            "no-gui", QCoreApplication::translate("main", "Load Console instead of GUI in case GUI is available"));
        parser.addOption(noGuiOption);
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
            Settings::I()
                ->setDefault("workdir.appdata", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
            Settings::I()->setDefault(
                "workdir.applocaldata", QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
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

        // Activate UI plugin
        UiPluginInterface* ui_plugin = nullptr;
        if( parser.isSet(noGuiOption) ) {
            LOG_D() << "aSocial - console mode";
            Settings::I()->setting("plugins.ui.console.active", true);
            Plugins::I()->settingActivePlugin("plugins.ui.console.active", "ui-console");
            Plugins::I()
                ->activateInterface("ui-console", QLatin1String("io.stateoftheart.asocial.plugin.UiPluginInterface"));
            ui_plugin = qobject_cast<UiPluginInterface*>(
                Plugins::I()->getPlugin("io.stateoftheart.asocial.plugin.UiPluginInterface", "ui-console"));
        } else {
            // Right now --gui option is not used, but later when the app will learn how to determine
            // which ui plugin to use - it will be necessary
            LOG_D() << "aSocial - GUI mode";
            Settings::I()->setting("plugins.ui.gui.active", true);
            Plugins::I()->settingActivePlugin("plugins.ui.gui.active", "ui-gui");
            Plugins::I()->activateInterface("ui-gui", QLatin1String("io.stateoftheart.asocial.plugin.UiPluginInterface"));
            ui_plugin = qobject_cast<UiPluginInterface*>(
                Plugins::I()->getPlugin("io.stateoftheart.asocial.plugin.UiPluginInterface", "ui-gui"));
        }

        if( ui_plugin == nullptr ) {
            LOG_F() << "Unable to locate UI plugin";
            return 1;
        }
        LOGGER->setSink(ui_plugin);
        ui_plugin->startUI();

        LOG_D() << "UI Plugin activated";

#if defined(Q_OS_UNIX) && !defined(Q_OS_ANDROID)
        // Handling SIGINT
        int sigFd[2] = {-1, -1};
        if( socketpair(AF_UNIX, SOCK_STREAM, 0, sigFd) == 0 ) {
            s_sigintFd = sigFd[0];
            struct sigaction act;
            memset(&act, 0, sizeof(act));
            act.sa_handler = sigintHandler;
            sigemptyset(&act.sa_mask);
            act.sa_flags = SA_RESTART;
            sigaction(SIGINT, &act, nullptr);

            QSocketNotifier* sn = new QSocketNotifier(sigFd[1], QSocketNotifier::Read, app.data());
            QObject::connect(sn, &QSocketNotifier::activated, app.data(), [sigFd, sn]() {
                char c;
                if( read(sigFd[1], &c, 1) == 1 )
                    QCoreApplication::quit();
                sn->setEnabled(false);
            });
        }
#endif

        QObject::connect(app.data(), &QCoreApplication::aboutToQuit, app.data(), []() { runShutdown(); });

        exitCode = app->exec();
    } catch( const std::exception& ex ) {
        LOG_F() << "Fatal error: unhandled std::exception in main()" << ex.what();
        try {
            runShutdown();
        } catch( ... ) {
            // Best-effort shutdown; ignore secondary failures
        }
        exitCode = EXIT_FAILURE;
    } catch( ... ) {
        LOG_F() << "Fatal error: unknown exception in main()";
        try {
            runShutdown();
        } catch( ... ) {
        }
        exitCode = EXIT_FAILURE;
    }

    return exitCode;
}
