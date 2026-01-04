#include "UiCmdPlugin.h"

#include <QLoggingCategory>

#include "uiworker.h"

Q_LOGGING_CATEGORY(C, PLUGIN_NAME)

Plugin* Plugin::s_pInstance = nullptr;

QString Plugin::name() const
{
    return QLatin1String(PLUGIN_NAME);
}

QStringList Plugin::requirements() const
{
    qCDebug(C) << __func__;
    return QStringList();
}

bool Plugin::init()
{
    if( isInitialized() )
        return true;

    qCDebug(C) << __func__;
    Plugin::s_pInstance = this;

    ui_thread = new UiWorker();
    ui_thread->setCore(m_core);

    configure();

    qCDebug(C) << "init() done";

    setInitialized(true);

    emit appNotice(this->name().append(" initialized"));

    return true;
}

bool Plugin::deinit()
{
    if( !isInitialized() )
        return true;
    qCDebug(C) << __func__;

    stopUI();

    delete ui_thread;

    Plugin::s_pInstance = nullptr;

    emit appNotice(this->name().append(" deinitialized"));
    qCDebug(C) << "deinit() done";
    setInitialized(false);

    return true;
}

bool Plugin::configure()
{
    qCDebug(C) << __func__;

    ui_thread->configure();

    return true;
}

bool Plugin::startUI()
{
    qCDebug(C) << __func__;

    ui_thread->start();

    return true;
}

bool Plugin::stopUI()
{
    qCDebug(C) << __func__;

    ui_thread->quit();
    ui_thread->wait();

    return true;
}
