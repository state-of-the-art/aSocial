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

#include "../src/console/CommandRouter.h"
#include "../src/console/SecureUtil.h"
#include "../src/console/Style.h"

/**
 * Unit tests for the ui-console plugin's console framework.
 *
 * Tests cover the components that can be exercised without a live
 * terminal: tokeniser, command routing, secure memory wipe, and
 * the style-gating mechanism.
 */
class TestUiConsole : public QObject
{
    Q_OBJECT

private slots:
    // --- CommandRouter::tokenize -------------------------------------------

    void testTokenizeSimple()
    {
        auto tokens = CommandRouter::tokenize(QStringLiteral("profile create mypass"));
        QCOMPARE(tokens.size(), 3);
        QCOMPARE(tokens[0], QStringLiteral("profile"));
        QCOMPARE(tokens[1], QStringLiteral("create"));
        QCOMPARE(tokens[2], QStringLiteral("mypass"));
    }

    void testTokenizeDoubleQuotes()
    {
        auto tokens = CommandRouter::tokenize(QStringLiteral("profile create mypass \"John Doe\""));
        QCOMPARE(tokens.size(), 4);
        QCOMPARE(tokens[3], QStringLiteral("John Doe"));
    }

    void testTokenizeSingleQuotes()
    {
        auto tokens = CommandRouter::tokenize(QStringLiteral("settings set key 'hello world'"));
        QCOMPARE(tokens.size(), 4);
        QCOMPARE(tokens[3], QStringLiteral("hello world"));
    }

    void testTokenizeBackslashEscape()
    {
        auto tokens = CommandRouter::tokenize(QStringLiteral("settings set key hello\\ world"));
        QCOMPARE(tokens.size(), 4);
        QCOMPARE(tokens[3], QStringLiteral("hello world"));
    }

    void testTokenizeEmpty()
    {
        auto tokens = CommandRouter::tokenize(QString());
        QVERIFY(tokens.isEmpty());
    }

    void testTokenizeWhitespaceOnly()
    {
        auto tokens = CommandRouter::tokenize(QStringLiteral("   "));
        QVERIFY(tokens.isEmpty());
    }

    // --- CommandRouter dispatch ---------------------------------------------

    void testDispatchLeafCommand()
    {
        CommandRouter router;
        bool called = false;
        QStringList receivedArgs;

        router.addCommand(
            QString(),
            QStringLiteral("hello"),
            QStringLiteral("Say hello"),
            {QStringLiteral("name")},
            [&](const QStringList& args) {
                called = true;
                receivedArgs = args;
            });

        auto result = router.dispatch(QStringLiteral("hello world"));
        QCOMPARE(result, CommandRouter::Result::Executed);
        QVERIFY(called);
        QCOMPARE(receivedArgs.size(), 1);
        QCOMPARE(receivedArgs[0], QStringLiteral("world"));
    }

    void testDispatchMenuNavigation()
    {
        CommandRouter router;
        router.addMenu(QString(), QStringLiteral("sub"), QStringLiteral("Submenu"));

        auto result = router.dispatch(QStringLiteral("sub"));
        QCOMPARE(result, CommandRouter::Result::NavigatedMenu);
        QCOMPARE(router.currentPath(), QStringLiteral("main/sub"));
    }

    void testDispatchNestedCommand()
    {
        CommandRouter router;
        router.addMenu(QString(), QStringLiteral("sub"), QStringLiteral("Submenu"));

        bool called = false;
        router.addCommand(
            QStringLiteral("sub"), QStringLiteral("action"), QStringLiteral("Do something"), {}, [&](const QStringList&) {
                called = true;
            });

        // Execute from root level without navigating first
        auto result = router.dispatch(QStringLiteral("sub action"));
        QCOMPARE(result, CommandRouter::Result::Executed);
        QVERIFY(called);
        // Should still be at root
        QCOMPARE(router.currentPath(), QStringLiteral("main"));
    }

    void testDispatchNavigateUp()
    {
        CommandRouter router;
        router.addMenu(QString(), QStringLiteral("sub"), QStringLiteral("Submenu"));

        router.dispatch(QStringLiteral("sub"));
        QCOMPARE(router.currentPath(), QStringLiteral("main/sub"));

        auto result = router.dispatch(QStringLiteral(".."));
        QCOMPARE(result, CommandRouter::Result::NavigatedUp);
        QCOMPARE(router.currentPath(), QStringLiteral("main"));
    }

    void testDispatchExit()
    {
        CommandRouter router;
        auto result = router.dispatch(QStringLiteral("exit"));
        QCOMPARE(result, CommandRouter::Result::Exit);
    }

    void testDispatchNotFound()
    {
        CommandRouter router;
        auto result = router.dispatch(QStringLiteral("nonexistent"));
        QCOMPARE(result, CommandRouter::Result::NotFound);
    }

    void testDispatchEmpty()
    {
        CommandRouter router;
        auto result = router.dispatch(QString());
        QCOMPARE(result, CommandRouter::Result::Empty);
    }

    // --- Completions -------------------------------------------------------

    void testCompletions()
    {
        CommandRouter router;
        router.addMenu(QString(), QStringLiteral("profile"), QStringLiteral("Profile"));
        router.addMenu(QString(), QStringLiteral("persona"), QStringLiteral("Persona"));
        router.addCommand(QString(), QStringLiteral("init"), QStringLiteral("Init"), {}, [](const QStringList&) {});

        auto all = router.completions(QString());
        QVERIFY(all.contains(QStringLiteral("profile")));
        QVERIFY(all.contains(QStringLiteral("persona")));
        QVERIFY(all.contains(QStringLiteral("init")));
        QVERIFY(all.contains(QStringLiteral("help")));
        QVERIFY(all.contains(QStringLiteral("exit")));

        auto p = router.completions(QStringLiteral("p"));
        QVERIFY(p.contains(QStringLiteral("profile")));
        QVERIFY(p.contains(QStringLiteral("persona")));
        QVERIFY(!p.contains(QStringLiteral("init")));
    }

    // --- SecureUtil ---------------------------------------------------------

    void testSecureWipeByteArray()
    {
        QByteArray buf("supersecret");
        QVERIFY(!buf.isEmpty());
        SecureUtil::wipe(buf);
        QVERIFY(buf.isEmpty());
    }

    void testSecureWipeString()
    {
        QString str = QStringLiteral("password123");
        QVERIFY(!str.isEmpty());
        SecureUtil::wipe(str);
        QVERIFY(str.isEmpty());
    }

    void testSecureWipeEmptyIsNoOp()
    {
        QByteArray buf;
        SecureUtil::wipe(buf);
        QVERIFY(buf.isEmpty());

        QString str;
        SecureUtil::wipe(str);
        QVERIFY(str.isEmpty());
    }

    void testSecureGuard()
    {
        QByteArray buf("sensitive");
        {
            SecureUtil::Guard guard(buf);
            QCOMPARE(buf, QByteArray("sensitive"));
        }
        QVERIFY(buf.isEmpty());
    }

    void testSecureStringGuard()
    {
        QString str = QStringLiteral("secret");
        {
            SecureUtil::StringGuard guard(str);
            QCOMPARE(str, QStringLiteral("secret"));
        }
        QVERIFY(str.isEmpty());
    }

    // --- Style gating ------------------------------------------------------

    void testStyleGatingOn()
    {
        Style::setEnabled(true);
        QVERIFY(Style::isEnabled());
        QVERIFY(QString::fromLatin1(Style::bold()).contains(QStringLiteral("\033[")));
        QVERIFY(QString::fromLatin1(Style::success()).contains(QStringLiteral("\033[")));
    }

    void testStyleGatingOff()
    {
        Style::setEnabled(false);
        QVERIFY(!Style::isEnabled());
        QCOMPARE(QString::fromLatin1(Style::bold()), QString());
        QCOMPARE(QString::fromLatin1(Style::reset()), QString());
        QCOMPARE(QString::fromLatin1(Style::success()), QString());
        Style::setEnabled(true); // restore
    }
};

QTEST_GUILESS_MAIN(TestUiConsole)
#include "plugin_ui_console.moc"
