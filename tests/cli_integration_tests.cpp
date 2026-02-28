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

#include "kotik/ProcessLogMonitor.h"
#include <QProcess>
#include <QTest>

class CliIntegrationTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() { /* global setup */ }

    void test_plugin_load_and_help_command()
    {
        QProcess app;
        app.setProgram("./aSocial-x86_64.AppImage");
        app.setArguments({"--appimage-extract-and-run"});

        ProcessLogMonitor* logs = new ProcessLogMonitor(&app, this);
        logs->clear();

        app.start();

        // Wait for "main>" prompt
        QVERIFY(logs->waitForLog("main>", 5000));
        // Log should not contain unsupported interface in plugin
        QVERIFY(!logs->contains("no supported interfaces found for plugin"));

        app.write("help\n");
        QVERIFY(logs->waitForLog("Commands available:", 500));
        QVERIFY(logs->contains("exit"));
        QVERIFY(logs->contains("Quit the session"));
        QVERIFY(logs->contains("history"));
        QVERIFY(logs->contains("Show the history"));
        QVERIFY(logs->contains("answer"));
        QVERIFY(logs->contains("Print the answer to Life, the Universe and Everything"));
        QVERIFY(logs->noErrorLogs());

        app.write("answer 42\n");
        QVERIFY(logs->waitForLog("The answer is: 42", 500));
        QVERIFY(logs->noErrorLogs());

        app.write("notacommand\n");
        QVERIFY(logs->waitForLog("Unknown command or incorrect parameters: notacommand.", 500));
        QVERIFY(logs->noErrorLogs());

        logs->contains(QRegularExpression("loaded in \\d+ ms"));

        app.write("exit\n");
        QVERIFY(app.waitForFinished(3000));
        QCOMPARE(app.exitCode(), 0);
    }
};

QTEST_MAIN(CliIntegrationTest)
#include "cli_integration_tests.moc"
