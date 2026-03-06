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

#include <QBuffer>
#include <QTemporaryDir>
#include <QTest>

#include "../src/RocksDBStore.h"

/**
 * Unit tests for RocksDBStore – exercises both filesystem mode and
 * QIODevice (VFS-compatible) mode.
 */
class TestDBKVRocksdb : public QObject
{
    Q_OBJECT

private slots:
    // --- Filesystem mode ---------------------------------------------------

    void testOpenAndClose()
    {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());

        RocksDBStore store;
        QVERIFY(!store.isOpen());
        QVERIFY(store.open(tmpDir.path() + "/testdb"));
        QVERIFY(store.isOpen());

        store.close();
        QVERIFY(!store.isOpen());
    }

    void testPutAndGet()
    {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());

        RocksDBStore store;
        QVERIFY(store.open(tmpDir.path() + "/testdb"));

        const QByteArray value = R"({"name":"Alice","age":30})";
        QVERIFY(store.put("user/1", value));

        QByteArray result;
        QVERIFY(store.get("user/1", result));
        QCOMPARE(result, value);

        store.close();
    }

    void testExists()
    {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());

        RocksDBStore store;
        QVERIFY(store.open(tmpDir.path() + "/testdb"));

        QVERIFY(!store.exists("missing"));
        QVERIFY(store.put("present", "data"));
        QVERIFY(store.exists("present"));

        store.close();
    }

    void testDelete()
    {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());

        RocksDBStore store;
        QVERIFY(store.open(tmpDir.path() + "/testdb"));

        QVERIFY(store.put("key", "val"));
        QVERIFY(store.exists("key"));
        QVERIFY(store.remove("key"));
        QVERIFY(!store.exists("key"));

        store.close();
    }

    void testListKeys()
    {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());

        RocksDBStore store;
        QVERIFY(store.open(tmpDir.path() + "/testdb"));

        QVERIFY(store.put("profile/alice", "{}"));
        QVERIFY(store.put("profile/bob", "{}"));
        QVERIFY(store.put("settings/theme", "dark"));

        QStringList profileKeys = store.listKeys("profile/");
        QCOMPARE(profileKeys.size(), 2);
        QVERIFY(profileKeys.contains("profile/alice"));
        QVERIFY(profileKeys.contains("profile/bob"));

        QStringList allKeys = store.listKeys("");
        QCOMPARE(allKeys.size(), 3);

        store.close();
    }

    void testPersistenceAcrossReopen()
    {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());
        const QString dbPath = tmpDir.path() + "/testdb";

        // Phase 1: write data
        {
            RocksDBStore store;
            QVERIFY(store.open(dbPath));
            QVERIFY(store.put("hello", "world"));
            store.close();
        }

        // Phase 2: reopen and verify
        {
            RocksDBStore store;
            QVERIFY(store.open(dbPath));
            QByteArray val;
            QVERIFY(store.get("hello", val));
            QCOMPARE(val, QByteArray("world"));
            store.close();
        }
    }

    // --- QIODevice mode (VFS-compatible) -----------------------------------

    void testDeviceOpenAndClose()
    {
        QBuffer device;
        device.open(QIODevice::ReadWrite);

        RocksDBStore store;
        QVERIFY(store.open(&device));
        QVERIFY(store.isOpen());

        store.close();
        QVERIFY(!store.isOpen());
    }

    void testDevicePutGetFlush()
    {
        QBuffer device;
        device.open(QIODevice::ReadWrite);

        // Phase 1: write and flush
        {
            RocksDBStore store;
            QVERIFY(store.open(&device));
            QVERIFY(store.put("key1", "value1"));
            QVERIFY(store.put("key2", "value2"));
            store.close(); // close implicitly flushes
        }

        QVERIFY(!device.data().isEmpty());

        // Phase 2: reopen from device and verify
        {
            RocksDBStore store;
            QVERIFY(store.open(&device));

            QByteArray val;
            QVERIFY(store.get("key1", val));
            QCOMPARE(val, QByteArray("value1"));

            QVERIFY(store.get("key2", val));
            QCOMPARE(val, QByteArray("value2"));

            store.close();
        }
    }

    void testDeviceListAndDelete()
    {
        QBuffer device;
        device.open(QIODevice::ReadWrite);

        RocksDBStore store;
        QVERIFY(store.open(&device));

        QVERIFY(store.put("a/1", "x"));
        QVERIFY(store.put("a/2", "y"));
        QVERIFY(store.put("b/1", "z"));

        QStringList aKeys = store.listKeys("a/");
        QCOMPARE(aKeys.size(), 2);

        QVERIFY(store.remove("a/1"));
        aKeys = store.listKeys("a/");
        QCOMPARE(aKeys.size(), 1);

        store.close();
    }

    void testDeviceRoundTrip()
    {
        QBuffer device;
        device.open(QIODevice::ReadWrite);

        // Write 100 entries, close, reopen, verify all
        const int N = 100;

        {
            RocksDBStore store;
            QVERIFY(store.open(&device));
            for( int i = 0; i < N; ++i ) {
                QString key = QStringLiteral("item/%1").arg(i, 4, 10, QLatin1Char('0'));
                QByteArray val = QStringLiteral("{\"idx\":%1}").arg(i).toUtf8();
                QVERIFY(store.put(key, val));
            }
            store.close();
        }

        {
            RocksDBStore store;
            QVERIFY(store.open(&device));
            QStringList keys = store.listKeys("item/");
            QCOMPARE(keys.size(), N);

            for( int i = 0; i < N; ++i ) {
                QString key = QStringLiteral("item/%1").arg(i, 4, 10, QLatin1Char('0'));
                QByteArray val;
                QVERIFY(store.get(key, val));
                QByteArray expected = QStringLiteral("{\"idx\":%1}").arg(i).toUtf8();
                QCOMPARE(val, expected);
            }
            store.close();
        }
    }

    void testEmptyDeviceOpen()
    {
        QBuffer device;
        device.open(QIODevice::ReadWrite);

        RocksDBStore store;
        QVERIFY(store.open(&device));
        QCOMPARE(store.listKeys("").size(), 0);
        store.close();
    }

    void testOverwriteExistingKey()
    {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());

        RocksDBStore store;
        QVERIFY(store.open(tmpDir.path() + "/testdb"));

        QVERIFY(store.put("key", "original"));
        QByteArray val;
        QVERIFY(store.get("key", val));
        QCOMPARE(val, QByteArray("original"));

        QVERIFY(store.put("key", "updated"));
        QVERIFY(store.get("key", val));
        QCOMPARE(val, QByteArray("updated"));

        store.close();
    }

    void testGetMissingKey()
    {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());

        RocksDBStore store;
        QVERIFY(store.open(tmpDir.path() + "/testdb"));

        QByteArray val;
        QVERIFY(!store.get("nonexistent", val));

        store.close();
    }
};

QTEST_GUILESS_MAIN(TestDBKVRocksdb)
#include "plugin_dbkv_rocksdb.moc"
