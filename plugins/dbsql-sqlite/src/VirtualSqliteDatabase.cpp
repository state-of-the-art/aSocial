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

#include "VirtualSqliteDatabase.h"

#include <QBuffer>
#include <QIODevice>
#include <QLoggingCategory>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QUuid>

#include <sodium.h>

Q_LOGGING_CATEGORY(VSD, "VirtualSqliteDatabase")

VirtualSqliteDatabase::VirtualSqliteDatabase(QObject* parent)
    : QObject(parent)
{}

VirtualSqliteDatabase::~VirtualSqliteDatabase()
{
    if( m_isOpen )
        close();
}

QSqlDatabase VirtualSqliteDatabase::open(QIODevice* device)
{
    if( m_isOpen )
        close();

    m_device = device;

    m_connectionName = QLatin1String("vfs_sqlite_") + QUuid::createUuid().toString(QUuid::WithoutBraces);

    QSqlDatabase db = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), m_connectionName);
    db.setDatabaseName(QLatin1String(":memory:"));

    if( !db.open() ) {
        qCCritical(VSD) << "Failed to open in-memory database:" << db.lastError().text();
        QSqlDatabase::removeDatabase(m_connectionName);
        return QSqlDatabase();
    }

    // Read existing data from the device
    if( m_device && m_device->isReadable() ) {
        m_device->seek(0);
        QByteArray existing = m_device->readAll();
        if( !existing.isEmpty() ) {
            if( !deserializeFromSql(existing) )
                qCWarning(VSD) << "Could not restore DB from device – starting fresh";
            secureWipe(existing);
        }
    }

    m_isOpen = true;

    m_autoFlushTimer.setInterval(30000);
    connect(&m_autoFlushTimer, &QTimer::timeout, this, &VirtualSqliteDatabase::flush);
    m_autoFlushTimer.start();

    qCDebug(VSD) << "Database opened on device" << m_device;
    return db;
}

QSqlDatabase VirtualSqliteDatabase::database() const
{
    return QSqlDatabase::database(m_connectionName, /* open = */ false);
}

void VirtualSqliteDatabase::flush()
{
    if( !m_isOpen || !m_device )
        return;

    if( !m_device->isOpen() || !m_device->isWritable() ) {
        qCWarning(VSD) << "Backing device not writable, skipping flush";
        return;
    }

    QByteArray sqlDump = serializeToSql();
    if( !sqlDump.isEmpty() ) {
        writeBackToDevice(sqlDump);
        secureWipe(sqlDump);
    }
}

void VirtualSqliteDatabase::close()
{
    if( !m_isOpen )
        return;

    m_autoFlushTimer.stop();
    flush();

    {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
        if( db.isOpen() )
            db.close();
    }
    QSqlDatabase::removeDatabase(m_connectionName);

    m_isOpen = false;
    m_device = nullptr;

    qCDebug(VSD) << "Database closed";
}

// ---------------------------------------------------------------------------
// Device I/O
// ---------------------------------------------------------------------------

void VirtualSqliteDatabase::writeBackToDevice(const QByteArray& data)
{
    m_device->seek(0);
    m_device->write(data);

    // Truncate any trailing bytes left from a previous (longer) dump.
    // QBuffer exposes the internal QByteArray directly.
    if( auto* buf = dynamic_cast<QBuffer*>(m_device) )
        buf->buffer().resize(static_cast<int>(m_device->pos()));
}

// ---------------------------------------------------------------------------
// Serialisation
// ---------------------------------------------------------------------------

QByteArray VirtualSqliteDatabase::serializeToSql() const
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
    if( !db.isOpen() )
        return {};

    QByteArray result;
    result.reserve(4096);

    QSqlQuery q(db);
    if( !q.exec(QLatin1String("SELECT type, name, sql FROM sqlite_master "
                              "WHERE sql IS NOT NULL ORDER BY rowid")) ) {
        qCWarning(VSD) << "Cannot read sqlite_master:" << q.lastError().text();
        return {};
    }

    result.append("BEGIN TRANSACTION;\n");

    QStringList tableNames;
    while( q.next() ) {
        const QString type = q.value(0).toString();
        const QString name = q.value(1).toString();
        const QString sql = q.value(2).toString();

        result.append(sql.toUtf8());
        result.append(";\n");

        if( type == QLatin1String("table") && !name.startsWith(QLatin1String("sqlite_")) )
            tableNames.append(name);
    }

    for( const QString& table : tableNames ) {
        QSqlQuery dq(db);
        if( !dq.exec(QStringLiteral("SELECT * FROM \"%1\"").arg(table)) )
            continue;

        const QSqlRecord rec = dq.record();
        const int colCount = rec.count();

        while( dq.next() ) {
            result.append(QStringLiteral("INSERT INTO \"%1\" VALUES(").arg(table).toUtf8());

            for( int i = 0; i < colCount; ++i ) {
                if( i > 0 )
                    result.append(',');

                const QVariant val = dq.value(i);
                if( val.isNull() ) {
                    result.append("NULL");
                } else if( val.typeId() == QMetaType::QByteArray ) {
                    result.append("X'");
                    result.append(val.toByteArray().toHex());
                    result.append('\'');
                } else if(
                    val.typeId() == QMetaType::Int || val.typeId() == QMetaType::LongLong
                    || val.typeId() == QMetaType::Double ) {
                    result.append(val.toString().toUtf8());
                } else {
                    QString s = val.toString();
                    s.replace(QLatin1Char('\''), QLatin1String("''"));
                    result.append('\'');
                    result.append(s.toUtf8());
                    result.append('\'');
                }
            }
            result.append(");\n");
        }
    }

    result.append("COMMIT;\n");
    return result;
}

bool VirtualSqliteDatabase::deserializeFromSql(const QByteArray& sqlDump)
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
    if( !db.isOpen() )
        return false;

    const QString sql = QString::fromUtf8(sqlDump);

    QStringList statements;
    QString current;
    bool inQuote = false;

    for( int i = 0; i < sql.length(); ++i ) {
        const QChar ch = sql[i];
        if( ch == QLatin1Char('\'') && !inQuote ) {
            inQuote = true;
            current += ch;
        } else if( ch == QLatin1Char('\'') && inQuote ) {
            if( i + 1 < sql.length() && sql[i + 1] == QLatin1Char('\'') ) {
                current += ch;
                current += sql[++i];
            } else {
                inQuote = false;
                current += ch;
            }
        } else if( ch == QLatin1Char(';') && !inQuote ) {
            const QString stmt = current.trimmed();
            if( !stmt.isEmpty() )
                statements.append(stmt);
            current.clear();
        } else {
            current += ch;
        }
    }

    QSqlQuery q(db);
    bool allOk = true;
    for( const QString& stmt : statements ) {
        if( !q.exec(stmt) ) {
            qCWarning(VSD) << "SQL restore failed:" << q.lastError().text() << "| stmt:" << stmt.left(120);
            allOk = false;
        }
    }
    return allOk;
}

void VirtualSqliteDatabase::secureWipe(QByteArray& data)
{
    if( !data.isEmpty() )
        sodium_memzero(data.data(), static_cast<size_t>(data.size()));
    data.clear();
}
