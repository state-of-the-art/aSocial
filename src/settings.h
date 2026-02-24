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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

class Settings : public QObject
{
    Q_OBJECT

public:
    inline static Settings *I()
    {
        if( s_pInstance == NULL )
            s_pInstance = new Settings();
        return s_pInstance;
    }
    inline static void destroyI() { delete s_pInstance; }

    inline static void setConfigFile(QString &path) { s_pInstance = new Settings(path); }

    Q_INVOKABLE QVariant setting(QString key, QVariant value = QVariant());
    Q_INVOKABLE bool isNull(QString key);
    void setDefault(QString key, QVariant value);

signals:
    void settingChanged(QString key);

private:
    explicit Settings(QObject *parent = 0);
    Settings(QString &path, QObject *parent = 0);
    ~Settings();

    QSettings m_settings;
    static Settings *s_pInstance;
};

#endif  // SETTINGS_H
