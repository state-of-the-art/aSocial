//#include <QtGui/QGuiApplication>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QLocale>
#include <QTranslator>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(Cm, "main")

#include "plugins.h"
#include "settings.h"

int main(int argc, char *argv[])
{
    qCDebug(Cm, "Init v%s", PROJECT_VERSION);

    QScopedPointer<QCoreApplication> app(new QCoreApplication(argc, argv));
    QCoreApplication::setOrganizationName("State Of The Art");
    QCoreApplication::setOrganizationDomain("state-of-the-art.io");
    QCoreApplication::setApplicationName("aSocial");
    QCoreApplication::setApplicationVersion(PROJECT_VERSION);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({
        {"no-gui", QCoreApplication::translate("main", "Do not load UI plugins")},
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

    qCDebug(Cm, "Init plugins...");
    Plugins::I();

    qCDebug(Cm, "Init settings...");
    Settings::I();

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
    qCDebug(Cm, "UI Plugin activated");

    return app->exec();
}
