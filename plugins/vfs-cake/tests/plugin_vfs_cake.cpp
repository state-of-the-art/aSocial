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
#include <QTemporaryDir>
#include <QScopedPointer>

#include "../src/EncryptedVFSContainer.h"

#include <sodium.h>

class TestVFSCake : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        QVERIFY(sodium_init() >= 0);
        QVERIFY(m_tempDir.isValid());
    }

    void testCreateContainer() {
        const QString path = m_tempDir.filePath(QLatin1String("new.vfs"));

        EncryptedVFSContainer c;
        QVERIFY(c.open(path, QLatin1String("password"), MAX_TEST_SIZE));
        QVERIFY(c.isOpen());

        QVERIFY(QFile::exists(path));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QVERIFY(f.size() >= static_cast<qint64>(EncryptedVFSContainer::DATA_START));
        f.close();

        c.close();
        QVERIFY(!c.isOpen());
    }
    void testOpenClose() {
        const QString path = m_tempDir.filePath(QLatin1String("oc.vfs"));

        EncryptedVFSContainer c;
        QVERIFY(c.open(path, QLatin1String("test"), MAX_TEST_SIZE));
        QVERIFY(c.isOpen());
        c.close();
        QVERIFY(!c.isOpen());
    }
    void testListFilesEmpty() {
        const QString path = m_tempDir.filePath(QLatin1String("empty.vfs"));

        EncryptedVFSContainer c;
        QVERIFY(c.open(path, QLatin1String("pw"), MAX_TEST_SIZE));
        QVERIFY(c.listFiles().isEmpty());
        c.close();
    }
    void testCreateAndListFile() {
        const QString path = m_tempDir.filePath(QLatin1String("list.vfs"));

        EncryptedVFSContainer c;
        QVERIFY(c.open(path, QLatin1String("pw"), MAX_TEST_SIZE));

        QIODevice* dev = c.openFile(QLatin1String("hello.txt"));
        QVERIFY(dev);
        dev->write("Hello World");
        dev->close();
        delete dev;

        QStringList files = c.listFiles();
        QCOMPARE(files.size(), 1);
        QCOMPARE(files.first(), QLatin1String("hello.txt"));

        c.close();
    }
    void testFileReadWriteRoundTrip() {
        const QString path = m_tempDir.filePath(QLatin1String("rw.vfs"));
        const QByteArray payload("Hello, encrypted VFS!");

        EncryptedVFSContainer c;
        QVERIFY(c.open(path, QLatin1String("pw"), MAX_TEST_SIZE));

        QIODevice* dev = c.openFile(QLatin1String("test.bin"));
        QVERIFY(dev);
        dev->write(payload);
        dev->close();
        delete dev;

        QByteArray readBack = c.readFile(QLatin1String("test.bin"));
        QCOMPARE(readBack, payload);

        c.close();
    }
    void testDeleteFile() {
        const QString path = m_tempDir.filePath(QLatin1String("del.vfs"));

        EncryptedVFSContainer c;
        QVERIFY(c.open(path, QLatin1String("pw"), MAX_TEST_SIZE));

        QIODevice* dev = c.openFile(QLatin1String("gone.txt"));
        QVERIFY(dev);
        dev->write("data");
        dev->close();
        delete dev;

        QCOMPARE(c.listFiles().size(), 1);
        QVERIFY(c.deleteFile(QLatin1String("gone.txt")));
        QVERIFY(c.listFiles().isEmpty());
        QVERIFY(c.readFile(QLatin1String("gone.txt")).isEmpty());

        c.close();
    }
    void testRenameFile() {
        const QString path = m_tempDir.filePath(QLatin1String("ren.vfs"));

        EncryptedVFSContainer c;
        QVERIFY(c.open(path, QLatin1String("pw"), MAX_TEST_SIZE));

        QIODevice* dev = c.openFile(QLatin1String("old.txt"));
        QVERIFY(dev);
        dev->write("content");
        dev->close();
        delete dev;

        QVERIFY(c.renameFile(QLatin1String("old.txt"), QLatin1String("new.txt")));
        QVERIFY(!c.listFiles().contains(QLatin1String("old.txt")));
        QVERIFY(c.listFiles().contains(QLatin1String("new.txt")));
        QCOMPARE(c.readFile(QLatin1String("new.txt")), QByteArray("content"));

        c.close();
    }
    void testMultipleFiles() {
        const QString path = m_tempDir.filePath(QLatin1String("multi.vfs"));

        EncryptedVFSContainer c;
        QVERIFY(c.open(path, QLatin1String("pw"), MAX_TEST_SIZE));

        for (int i = 0; i < 5; ++i) {
            const QString name = QStringLiteral("file_%1.dat").arg(i);
            QIODevice* dev = c.openFile(name);
            QVERIFY(dev);
            dev->write(QByteArray::number(i).repeated(10));
            dev->close();
            delete dev;
        }

        QCOMPARE(c.listFiles().size(), 5);

        for (int i = 0; i < 5; ++i) {
            const QString name = QStringLiteral("file_%1.dat").arg(i);
            QCOMPARE(c.readFile(name), QByteArray::number(i).repeated(10));
        }

        c.close();
    }
    void testAutoGrowth() {
        const QString path = m_tempDir.filePath(QLatin1String("grow.vfs"));

        EncryptedVFSContainer c;
        QVERIFY(c.open(path, QLatin1String("pw"), MAX_TEST_SIZE));
        const quint64 initialSize = c.containerSize();

        const QByteArray largePayload(1024 * 1024, 'A');
        QIODevice* dev = c.openFile(QLatin1String("big.bin"));
        QVERIFY(dev);
        dev->write(largePayload);
        dev->close();
        delete dev;

        QVERIFY(c.containerSize() > initialSize);
        QCOMPARE(c.readFile(QLatin1String("big.bin")), largePayload);

        c.close();
    }
    void testDemoMode() {
        const QString path = m_tempDir.filePath(QLatin1String("demo.vfs"));

        EncryptedVFSContainer c;
        QVERIFY(c.open(path, QString(), MAX_TEST_SIZE));

        QIODevice* dev = c.openFile(QLatin1String("demo.txt"));
        QVERIFY(dev);
        dev->write("demo data");
        dev->close();
        delete dev;

        QCOMPARE(c.readFile(QLatin1String("demo.txt")), QByteArray("demo data"));
        c.close();

        EncryptedVFSContainer c2;
        QVERIFY(c2.open(path, QString()));
        QVERIFY(c2.listFiles().contains(QLatin1String("demo.txt")));
        c2.close();
    }
    void testDifferentKeysIsolation() {
        const QString path = m_tempDir.filePath(QLatin1String("iso.vfs"));

        {
            EncryptedVFSContainer c;
            QVERIFY(c.open(path, QLatin1String("alpha"), MAX_TEST_SIZE));
            QIODevice* dev = c.openFile(QLatin1String("secret.txt"));
            QVERIFY(dev);
            dev->write("alpha data");
            dev->close();
            delete dev;
            c.close();
        }

        {
            EncryptedVFSContainer c;
            QVERIFY(c.open(path, QLatin1String("beta")));
            QVERIFY(c.listFiles().isEmpty());
            c.close();
        }

        {
            EncryptedVFSContainer c;
            QVERIFY(c.open(path, QLatin1String("alpha")));
            QVERIFY(c.listFiles().contains(QLatin1String("secret.txt")));
            QCOMPARE(c.readFile(QLatin1String("secret.txt")), QByteArray("alpha data"));
            c.close();
        }
    }
    void testPersistenceAcrossReopen() {
        const QString path = m_tempDir.filePath(QLatin1String("persist.vfs"));

        {
            EncryptedVFSContainer c;
            QVERIFY(c.open(path, QLatin1String("pw"), MAX_TEST_SIZE));
            QIODevice* dev = c.openFile(QLatin1String("keep.txt"));
            QVERIFY(dev);
            dev->write("persistent");
            dev->close();
            delete dev;
            c.close();
        }

        {
            EncryptedVFSContainer c;
            QVERIFY(c.open(path, QLatin1String("pw")));
            QStringList files = c.listFiles();
            QCOMPARE(files.size(), 1);
            QCOMPARE(files.first(), QLatin1String("keep.txt"));
            QCOMPARE(c.readFile(QLatin1String("keep.txt")), QByteArray("persistent"));
            c.close();
        }
    }

private:
    static constexpr quint64 MAX_TEST_SIZE = 2 * 1024 * 1024; // 2 MiB
    QTemporaryDir m_tempDir;
};

QTEST_GUILESS_MAIN(TestVFSCake)
#include "plugin_vfs_cake.moc"
