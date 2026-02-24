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

#include <QTest>
#include <QBuffer>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

#include "../src/VirtualSqliteDatabase.h"

#include <sodium.h>

/**
 * Tests for VirtualSqliteDatabase using a plain QBuffer as backing store,
 * proving the DB plugin is completely decoupled from any encryption layer.
 */
class TestDBSQLSqlite : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        QVERIFY(sodium_init() >= 0);
    }

    void testOpenAndClose() {
        QBuffer device;
        device.open(QIODevice::ReadWrite);

        VirtualSqliteDatabase db;
        QSqlDatabase handle = db.open(&device);
        QVERIFY(handle.isValid());
        QVERIFY(handle.isOpen());
        QVERIFY(db.isOpen());

        db.close();
        QVERIFY(!db.isOpen());
    }
    void testCreateTableAndInsert() {
        QBuffer device;
        device.open(QIODevice::ReadWrite);

        VirtualSqliteDatabase db;
        QSqlDatabase handle = db.open(&device);
        QVERIFY(handle.isValid());

        QSqlQuery q(handle);
        QVERIFY(q.exec(QLatin1String("CREATE TABLE kv (key TEXT PRIMARY KEY, value TEXT)")));
        QVERIFY(q.exec(QLatin1String("INSERT INTO kv VALUES ('greeting', 'hello')")));

        QVERIFY(q.exec(QLatin1String("SELECT value FROM kv WHERE key='greeting'")));
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(), QLatin1String("hello"));

        db.close();
    }
    void testFlushAndReopen() {
        QBuffer device;
        device.open(QIODevice::ReadWrite);

        // Phase 1 – create and populate
        {
            VirtualSqliteDatabase db;
            QSqlDatabase handle = db.open(&device);
            QVERIFY(handle.isValid());

            QSqlQuery q(handle);
            QVERIFY(q.exec(QLatin1String(
                "CREATE TABLE kv (key TEXT PRIMARY KEY, value TEXT)")));
            QVERIFY(q.exec(QLatin1String(
                "INSERT INTO kv VALUES ('greeting', 'hello')")));
            QVERIFY(q.exec(QLatin1String(
                "INSERT INTO kv VALUES ('farewell', 'goodbye')")));

            db.close(); // flushes to device
        }

        QVERIFY(!device.data().isEmpty());

        // Phase 2 – reopen from the same device and verify
        {
            VirtualSqliteDatabase db;
            QSqlDatabase handle = db.open(&device);
            QVERIFY(handle.isValid());

            QSqlQuery q(handle);
            QVERIFY(q.exec(QLatin1String("SELECT value FROM kv WHERE key='greeting'")));
            QVERIFY(q.next());
            QCOMPARE(q.value(0).toString(), QLatin1String("hello"));

            QVERIFY(q.exec(QLatin1String("SELECT value FROM kv WHERE key='farewell'")));
            QVERIFY(q.next());
            QCOMPARE(q.value(0).toString(), QLatin1String("goodbye"));

            db.close();
        }
    }
    void testBlobStorage() {
        QBuffer device;
        device.open(QIODevice::ReadWrite);

        const QByteArray blobData(256, '\xAB');

        // Phase 1 – insert BLOB
        {
            VirtualSqliteDatabase db;
            QSqlDatabase handle = db.open(&device);
            QVERIFY(handle.isValid());

            QSqlQuery q(handle);
            QVERIFY(q.exec(QLatin1String(
                "CREATE TABLE blobs (id INTEGER PRIMARY KEY, data BLOB)")));

            q.prepare(QLatin1String("INSERT INTO blobs (id, data) VALUES (1, :data)"));
            q.bindValue(QLatin1String(":data"), blobData);
            QVERIFY(q.exec());

            db.close();
        }

        // Phase 2 – read BLOB back
        {
            VirtualSqliteDatabase db;
            QSqlDatabase handle = db.open(&device);
            QVERIFY(handle.isValid());

            QSqlQuery q(handle);
            QVERIFY(q.exec(QLatin1String("SELECT data FROM blobs WHERE id=1")));
            QVERIFY(q.next());
            QCOMPARE(q.value(0).toByteArray(), blobData);

            db.close();
        }
    }
    void testMultipleTables() {
        QBuffer device;
        device.open(QIODevice::ReadWrite);

        VirtualSqliteDatabase db;
        QSqlDatabase handle = db.open(&device);
        QVERIFY(handle.isValid());

        QSqlQuery q(handle);
        QVERIFY(q.exec(QLatin1String("CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT)")));
        QVERIFY(q.exec(QLatin1String("CREATE TABLE posts (id INTEGER PRIMARY KEY, body TEXT)")));
        QVERIFY(q.exec(QLatin1String("INSERT INTO users VALUES (1, 'Alice')")));
        QVERIFY(q.exec(QLatin1String("INSERT INTO posts VALUES (1, 'Hello world')")));

        QVERIFY(q.exec(QLatin1String("SELECT name FROM users WHERE id=1")));
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(), QLatin1String("Alice"));

        QVERIFY(q.exec(QLatin1String("SELECT body FROM posts WHERE id=1")));
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(), QLatin1String("Hello world"));

        db.close();
    }
};

QTEST_GUILESS_MAIN(TestDBSQLSqlite)
#include "plugin_dbsql_sqlite.moc"
