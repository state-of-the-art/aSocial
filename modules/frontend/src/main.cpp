#include <QDateTime>
#include <QFile>
#include <QGuiApplication>
#include <cstdio>

#include "frontend/frontend.h"
#include "settings.h"

#ifdef Q_OS_ANDROID
#include <android/log.h>
#endif

void printLogMessage(int priority, QString msg)
{
#ifdef Q_OS_ANDROID
    __android_log_print(priority, "asocial", msg.toLocal8Bit().constData());
#else
    ::std::fprintf(stderr, "Error: %s", msg.toLocal8Bit().constData());
#endif
}

void myMessageOutput(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    switch( type ) {
        case QtDebugMsg:
            printLogMessage(3, QString("[aSocial %1] %2\n")
                                   .arg(QDateTime::currentDateTime().toString("dd.MM.yy hh:mm:ss.zzz"))
                                   .arg(msg));
            break;
        case QtWarningMsg:
            printLogMessage(5, QString("[aSocial] Warning: %1\n").arg(msg));
            break;
        case QtCriticalMsg:
            printLogMessage(6, QString("[aSocial] Critical: %1\n").arg(msg));
            break;
        case QtFatalMsg:
            printLogMessage(7, QString("[aSocial] Fatal: %1\n").arg(msg));
            Frontend::destroyI();
            abort();
    }
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageOutput);

    QGuiApplication app(argc, argv);

    // Last argument is a configuration file
    if( app.arguments().count() > 1 && QFile::exists(app.arguments().last()) )
        Settings::setConfigFile(app.arguments().last());

    Frontend::I()->init(&app);

    qDebug("Application init done, starting");

    return app.exec();
}
