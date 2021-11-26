#include "settings.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CS, "Settings")

Settings *Settings::s_pInstance = NULL;

Settings::Settings(QObject *parent)
    : QObject(parent)
    , m_settings()
{
    qCDebug(CS, "Create object");
}

Settings::Settings(QString &path, QObject *parent)
    : QObject(parent)
    , m_settings(path, QSettings::IniFormat)
{
    qCDebug(CS) << "Create Settings from file" << path;
}

Settings::~Settings()
{
    qCDebug(CS, "Destroy Settings");
}

QVariant Settings::setting(QString key, QVariant value)
{
    if( ! value.isNull() ) {
        m_settings.setValue(key, value);
        emit settingChanged(key);
    }
    if( m_settings.value(key).isNull() ) {
        qCWarning(CS) << "Unable to find predefined setting" << key;
    }

    return m_settings.value(key);
}

bool Settings::isNull(QString key)
{
    return m_settings.value(key).isNull();
}

void Settings::setDefault(QString key, QVariant value)
{
    if( m_settings.value(key).isNull() ) {
        qCDebug(CS) << "Set default value for" << key << "=" << value;
        m_settings.setValue(key, value);
        emit settingChanged(key);
    }
}
