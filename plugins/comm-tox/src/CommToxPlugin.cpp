#include "CommToxPlugin.h"

#include <QLoggingCategory>
#include "tox/tox.h"

Q_LOGGING_CATEGORY(C, PLUGIN_NAME)

Plugin* Plugin::s_pInstance = nullptr;

QString Plugin::name() const
{
    qCDebug(C) << "Tox version:" << tox_version_major() << tox_version_minor() << tox_version_patch();
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

    // TODO: Init here

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

    // TODO: Deinit here

    Plugin::s_pInstance = nullptr;

    emit appNotice(this->name().append(" deinitialized"));
    qCDebug(C) << "deinit() done";
    setInitialized(false);

    return true;
}

bool Plugin::configure()
{
    qCDebug(C) << __func__;
    return true;
}
