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

#include "settings.h"

#include "Log.h"

Settings* Settings::s_pInstance = NULL;

Settings::Settings(QObject* parent)
    : QObject(parent)
    , m_settings()
{
    LOG_D() << "Create object";
}

Settings::Settings(QString& path, QObject* parent)
    : QObject(parent)
    , m_settings(path, QSettings::IniFormat)
{
    LOG_D() << "Create Settings from file" << path;
}

Settings::~Settings()
{
    LOG_D() << "Destroy Settings";
}

QVariant Settings::setting(QString key, QVariant value)
{
    if( !value.isNull() ) {
        m_settings.setValue(key, value);
        emit settingChanged(key);
    }
    if( m_settings.value(key).isNull() ) {
        LOG_W() << "Unable to find predefined setting" << key;
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
        LOG_D() << "Set default value for" << key << "=" << value;
        m_settings.setValue(key, value);
        emit settingChanged(key);
    }
}

QStringList Settings::listAllKeys() const
{
    return m_settings.allKeys();
}
