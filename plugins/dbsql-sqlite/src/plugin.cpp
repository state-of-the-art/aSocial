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

#include "plugin.h"

#include <sodium.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(C, PLUGIN_NAME)

Plugin* Plugin::s_pInstance = nullptr;

Plugin::Plugin() = default;
Plugin::~Plugin() = default;

QString Plugin::name() const
{
    return QLatin1String(PLUGIN_NAME);
}

QStringList Plugin::requirements() const
{
    qCDebug(C) << __func__;
    return {};
}

bool Plugin::init()
{
    if( isInitialized() )
        return true;

    qCDebug(C) << __func__;
    Plugin::s_pInstance = this;

    if( sodium_init() < 0 ) {
        qCCritical(C) << "libsodium initialization failed";
        emit appError(name() + QLatin1String(": libsodium init failed"));
        return false;
    }

    setInitialized(true);
    emit appNotice(name().append(QLatin1String(" initialized")));
    return true;
}

bool Plugin::deinit()
{
    if( !isInitialized() )
        return true;
    qCDebug(C) << __func__;

    if( m_db.isOpen() )
        m_db.close();

    Plugin::s_pInstance = nullptr;
    emit appNotice(name().append(QLatin1String(" deinitialized")));
    setInitialized(false);
    return true;
}

bool Plugin::configure()
{
    qCDebug(C) << __func__;
    return true;
}

// ---------------------------------------------------------------------------
// DBSQLPluginInterface
// ---------------------------------------------------------------------------

QSqlDatabase Plugin::openDatabaseFile(QIODevice* device)
{
    return m_db.open(device);
}

void Plugin::flushDatabase()
{
    m_db.flush();
}

void Plugin::closeDatabase()
{
    m_db.close();
}
