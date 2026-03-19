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

#include "kotik/Kotik.h"
#include <algorithm>
#include <QTest>

class ConsoleIntegrationTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {}

    /**
     * @brief Verify plugin loads, help command works, error handling is sane.
     */
    void test_plugin_load_and_help_command()
    {
        Kotik* k = new Kotik(this);
        QVERIFY(k->waitForLog("main (no profile) >", 5000));
        QVERIFY(!k->contains("no supported interfaces found for plugin"));
        QVERIFY(k->contains("Create Settings from file \"" + k->workdirPath()));

        k->write("help");
        QVERIFY(k->waitForLog("Commands:", 500));
        QVERIFY(k->waitForLog("exit", 500));
        QVERIFY(k->contains("Exit aSocial"));

        QVERIFY(k->contains("settings"));
        QVERIFY(k->contains("profile"));
        QVERIFY(k->contains("persona"));
        QVERIFY(k->contains("contact"));
        QVERIFY(k->contains("group"));
        QVERIFY(k->contains("message"));
        QVERIFY(k->contains("event"));
        QVERIFY(k->contains("init"));
        QVERIFY(k->noErrorLogs());

        k->write("notacommand");
        QVERIFY(k->waitForLog("Unknown command or incorrect parameters: notacommand.", 500));

        k->contains(QRegularExpression("loaded in \\d+ ms"));

        QVERIFY(k->noErrorLogs());
        QVERIFY(k->exitApp());
        QCOMPARE(k->exitCode(), 0);
    }

    /**
     * @brief Test settings list / get / set commands.
     */
    void test_settings_commands()
    {
        Kotik* k = new Kotik(this);
        QVERIFY(k->waitForLog("main (no profile) >", 5000));

        k->write("settings list");
        QVERIFY(k->waitForLog("Settings", 500));
        QVERIFY(k->waitForLog("vfs.container.path", 500));
        QVERIFY(k->waitForLog("workdir.appdata = " + k->workdirPath(), 500));
        QVERIFY(k->waitForLog("workdir.applocaldata = " + k->workdirPath(), 500));

        k->clear();
        k->write("settings get vfs.plugin");
        QVERIFY(k->waitForLog("vfs-cake", 500));

        k->clear();
        k->write("settings set test.key hello");
        QVERIFY(k->waitForLog("test.key = hello", 500));

        k->clear();
        k->write("settings get test.key");
        QVERIFY(k->waitForLog("hello", 500));

        QVERIFY(k->noErrorLogs());
        QVERIFY(k->exitApp());
        QCOMPARE(k->exitCode(), 0);
    }

    /**
     * @brief Test the init -> create -> current -> close -> open cycle.
     */
    void test_profile_lifecycle()
    {
        Kotik* k = new Kotik(this);
        QVERIFY(k->waitForLog("main (no profile) >", 5000));

        k->clear();
        k->write("init 1");
        QVERIFY(k->waitForLog("Container created at", 15000));

        k->clear();
        k->write("profile create");
        QVERIFY(k->waitForLog("Enter password:", 500));
        k->clear();
        k->write("testpass");
        QVERIFY(k->waitForLog("Confirm password:", 500));
        k->clear();
        k->write("testpass");
        QVERIFY(k->waitForLog("Display name:", 500));
        k->clear();
        k->write("TestUser");
        QVERIFY(k->waitForLog("Profile created: TestUser", 1000));
        QVERIFY(k->waitForLog("main (TestUser) >", 500));

        k->clear();
        k->write("profile current");
        QVERIFY(k->waitForLog("Current profile:", 500));
        QVERIFY(k->contains("TestUser"));

        k->clear();
        k->write("profile close");
        QVERIFY(k->waitForLog("Profile closed", 500));
        QVERIFY(k->waitForLog("main (no profile) >", 500));

        k->clear();
        k->write("profile open testpass");
        QVERIFY(k->waitForLog("Profile opened: TestUser", 5000));

        k->write("profile close");
        QVERIFY(k->waitForLog("Profile closed", 500));

        QVERIFY(k->noErrorLogs());
        QVERIFY(k->exitApp());
        QCOMPARE(k->exitCode(), 0);

        // Reopening the existing profile
        Kotik* k2 = new Kotik(this, false);
        k2->setWorkdir(k->workdirPath());
        k2->start();
        QVERIFY(k2->waitForLog("main (no profile) >", 5000));

        k2->write("profile open");
        QVERIFY(k2->waitForLog("Enter password:", 5000));
        k2->write("testpass");
        QVERIFY(k2->waitForLog("Profile opened: TestUser", 5000));
        QVERIFY(k2->contains("Container opened: \"" + k2->workdirFilePath("data.vfs")));
        QVERIFY(!k2->contains("testpass"));

        QVERIFY(k2->noErrorLogs());
        QVERIFY(k2->exitApp());
        QCOMPARE(k2->exitCode(), 0);
    }

    /**
     * @brief Test contact CRUD within an open profile.
     */
    void test_contact_crud()
    {
        Kotik* k = new Kotik(this);
        QVERIFY(k->waitForLog("main (no profile) >", 5000));

        k->write("init 1");
        QVERIFY(k->waitForLog("Container created at", 15000));
        k->write("profile create pw Alice");
        QVERIFY(k->waitForLog("Profile created:", 5000));

        k->clear();
        k->write("contact add Bob");
        QVERIFY(k->waitForLog("Contact added: Bob", 500));
        QString uid;
        QVERIFY(k->findValues("Bob (uid=%s)", &uid));

        k->clear();
        k->write("contact list");
        QVERIFY(k->waitForLog("Contacts (1):", 500));
        QVERIFY(k->waitForLog("Bob", 500));
        QVERIFY(k->contains("trust=0"));
        QVERIFY(k->contains(QString("uid=%1").arg(uid)));

        k->clear();
        k->write("contact info");
        QVERIFY(k->waitForLog("Contact UID:", 500));
        k->write(uid);
        QVERIFY(k->waitForLog("Contact:", 500));
        QVERIFY(k->waitForLog("Name:  Bob", 500));

        k->clear();
        k->write("contact search Bob");
        QVERIFY(k->waitForLog("Found 1 contact", 500));
        QVERIFY(k->waitForLog("Bob", 500));
        QVERIFY(k->contains(QString("uid=%1").arg(uid)));

        k->clear();
        k->write("message send Bob Test by unique name");
        QVERIFY(k->waitForLog("Message sent", 500));

        k->clear();
        k->write("contact add Bob");
        QVERIFY(k->waitForLog("Contact added: Bob", 500));

        k->clear();
        k->write("message send Bob");
        QVERIFY(k->waitForLog("Message failed: too many contacts matches 'Bob'", 500));

        k->clear();
        k->write("message send");
        QVERIFY(k->waitForLog("Recipient UID:", 500));
        k->write(uid);
        QVERIFY(k->waitForLog("Message body:", 500));
        k->write("Test by uid");
        QVERIFY(k->waitForLog("Message sent", 500));

        k->clear();
        k->write(QString("contact delete %1").arg(uid));
        QVERIFY(k->waitForLog("Delete this contact?", 500));
        k->write("y");
        QVERIFY(k->waitForLog("Contact deleted", 500));

        QVERIFY(k->noErrorLogs());
        QVERIFY(k->exitApp());
        QCOMPARE(k->exitCode(), 0);
    }

    /**
     * @brief Test group and message commands.
     */
    void test_group_and_message()
    {
        Kotik* k = new Kotik(this);
        QVERIFY(k->waitForLog("main (no profile) >", 5000));

        k->write("init 1");
        QVERIFY(k->waitForLog("Container created at", 15000));
        k->write("profile create pw2 GroupTester");
        QVERIFY(k->waitForLog("Profile created:", 15000));

        k->clear();
        k->write("group create Family");
        QVERIFY(k->waitForLog("Group created: Family", 500));
        QString uid;
        QVERIFY(k->findValues("Family (uid=%s)", &uid));

        k->clear();
        k->write("group list");
        QVERIFY(k->waitForLog("Groups (1):", 500));
        QVERIFY(k->waitForLog("Family", 500));
        QVERIFY(k->contains(QString("uid=%1").arg(uid)));

        k->clear();
        k->write("message send nobody Hello_World");
        QVERIFY(k->waitForLog("Message failed: nobody not found", 500));

        k->clear();
        k->write("message list");
        QVERIFY(k->waitForLog("Messages (1):", 500));
        QVERIFY(k->contains("Hello_World"));

        QVERIFY(k->noErrorLogs());
        QVERIFY(k->exitApp());
        QCOMPARE(k->exitCode(), 0);
    }

    /**
     * @brief Test persona and event commands.
     */
    void test_persona_and_events()
    {
        Kotik* k = new Kotik(this);
        QVERIFY(k->waitForLog("main (no profile) >", 5000));

        k->write("init 1");
        QVERIFY(k->waitForLog("Container created at", 15000));
        k->write("profile create pw3 PersonaTester");
        QVERIFY(k->waitForLog("Profile created:", 15000));

        k->clear();
        k->write("persona list");
        QVERIFY(k->waitForLog("Personas (1):", 500));
        QVERIFY(k->waitForLog("[default]", 500));

        k->clear();
        k->write("persona create WorkSelf");
        QVERIFY(k->waitForLog("Persona created: WorkSelf", 500));

        k->clear();
        k->write("persona list");
        QVERIFY(k->waitForLog("Personas (2):", 500));

        k->clear();
        k->write("event create Birthday 2026-03-15");
        QVERIFY(k->waitForLog("Event created: Birthday", 500));

        k->clear();
        k->write("event list");
        QVERIFY(k->waitForLog("Events (1):", 500));
        QVERIFY(k->waitForLog("Birthday", 500));
        QVERIFY(k->contains("2026-03-15T00:00:00"));

        QVERIFY(k->noErrorLogs());
        QVERIFY(k->exitApp());
        QCOMPARE(k->exitCode(), 0);
    }

    /**
     * @brief Test profile export and import.
     */
    void test_export_import()
    {
        Kotik* k = new Kotik(this);
        QVERIFY(k->waitForLog("main (no profile) >", 5000));

        k->write("init 1");
        QVERIFY(k->waitForLog("Container created at", 15000));
        k->write("profile create pw4 Exporter");
        QVERIFY(k->waitForLog("Profile created:", 15000));

        k->write("contact add Carol");
        QVERIFY(k->waitForLog("Contact added", 500));

        k->clear();
        k->write("profile export");
        QVERIFY(k->waitForLog("Exported profile data", 2000));

        QVERIFY(k->noErrorLogs());
        QVERIFY(k->exitApp());
        QCOMPARE(k->exitCode(), 0);
    }

    /**
     * @brief Test profile deletion with secure wipe.
     */
    void test_profile_delete()
    {
        Kotik* k = new Kotik(this);
        QVERIFY(k->waitForLog("main (no profile) >", 5000));

        k->write("init 1");
        QVERIFY(k->waitForLog("Container created at", 15000));
        k->write("profile create pw5 DeleteMe");
        QVERIFY(k->waitForLog("Profile created:", 15000));

        k->clear();
        k->write("profile delete");
        QVERIFY(k->waitForLog("Securely delete this profile? This cannot be undone (y/N)", 500));
        k->write("y");
        QVERIFY(k->waitForLog("Profile deleted and securely wiped", 5000));
        QVERIFY(k->waitForLog("main (no profile) >", 500));

        QVERIFY(k->noErrorLogs());
        QVERIFY(k->exitApp());
        QCOMPARE(k->exitCode(), 0);
    }

    /**
     * @brief Verify commands fail gracefully when no profile is open.
     */
    void test_no_profile_guard()
    {
        Kotik* k = new Kotik(this);
        QVERIFY(k->waitForLog("main (no profile) >", 5000));

        k->write("contact list");
        QVERIFY(k->waitForLog("No profile open", 500));

        k->clear();
        k->write("group list");
        QVERIFY(k->waitForLog("No profile open", 500));

        k->clear();
        k->write("message list");
        QVERIFY(k->waitForLog("No profile open", 500));

        k->clear();
        k->write("event list");
        QVERIFY(k->waitForLog("No profile open", 500));

        k->clear();
        k->write("persona list");
        QVERIFY(k->waitForLog("No profile open", 500));

        k->clear();
        k->write("profile current");
        QVERIFY(k->waitForLog("No profile is currently open", 500));

        QVERIFY(k->noErrorLogs());
        QVERIFY(k->exitApp());
        QCOMPARE(k->exitCode(), 0);
    }
};

QTEST_MAIN(ConsoleIntegrationTest)
#include "console_integration_tests.moc"
