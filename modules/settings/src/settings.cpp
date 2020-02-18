#include "settings.h"

#include <QDebug>

Settings *Settings::s_pInstance = NULL;

Settings::Settings(QObject *parent)
    : QObject(parent)
    , m_settings()
{
    qDebug("Create Settings");
}

Settings::Settings(QString &path, QObject *parent)
    : QObject(parent)
    , m_settings(path, QSettings::IniFormat)
{
    qDebug() << "Create Settings from file" << path;
}

Settings::~Settings()
{
    qDebug("Destroy Settings");
}

QVariant Settings::setting(QString key, QVariant value)
{
    if( ! value.isNull() ) {
        m_settings.setValue(key, value);
        emit settingChanged(key);
    }
    if( m_settings.value(key).isNull() ) {
        qWarning() << "Unable to find predefined setting" << key;
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
        qDebug() << "Set default value for" << key << "=" << value;
        m_settings.setValue(key, value);
        emit settingChanged(key);
    }
}
