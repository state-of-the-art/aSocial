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

#ifndef VIRTUALSQLITEDATABASE_H
#define VIRTUALSQLITEDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QTimer>

/**
 * @brief In-memory SQLite database backed by any seekable QIODevice.
 *
 * On open() the device is read in full and its SQL-dump content is replayed
 * into an in-memory database.  All SQL operations happen in RAM via the
 * standard Qt QSqlDatabase API.  On flush() or close() the database is
 * serialised back to SQL statements and written to the device.
 *
 * The device is NOT owned by this class; it must remain open and accessible
 * for the lifetime of the database.
 *
 * An auto-flush timer fires every 30 seconds.
 *
 * @note Thread safety: use from a single thread only.
 */
class VirtualSqliteDatabase : public QObject
{
    Q_OBJECT

public:
    explicit VirtualSqliteDatabase(QObject* parent = nullptr);
    ~VirtualSqliteDatabase() override;

    /**
     * @brief Open (or create) a database backed by @p device.
     * @param device  Readable + writable + seekable QIODevice (not owned).
     * @return A ready-to-use in-memory QSqlDatabase handle, or an invalid
     *         QSqlDatabase on failure.
     */
    QSqlDatabase open(QIODevice* device);

    /**
     * @brief Return the current in-memory database handle.
     */
    QSqlDatabase database() const;

    /**
     * @brief Serialise the in-memory database back to the device.
     */
    void flush();

    /**
     * @brief Flush, close the database, and release the device reference.
     */
    void close();

    bool isOpen() const { return m_isOpen; }

private:
    QByteArray serializeToSql() const;
    bool       deserializeFromSql(const QByteArray& sqlDump);
    void       writeBackToDevice(const QByteArray& data);
    static void secureWipe(QByteArray& data);

    QIODevice* m_device = nullptr;
    QString    m_connectionName;
    QTimer     m_autoFlushTimer;
    bool       m_isOpen = false;
};

#endif // VIRTUALSQLITEDATABASE_H
