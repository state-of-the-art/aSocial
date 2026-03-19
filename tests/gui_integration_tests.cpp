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

class GuiIntegrationTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {}

    /**
     * @brief Verify application shuts down gracefully in gui mode (no segfault, exit code 0).
     */
    void test_gui_graceful_shutdown()
    {
        Kotik* k = new Kotik(this, false);
        k->setEnv({{"QT_QPA_PLATFORM", "offscreen"}});
        k->start({"--gui"});
        QVERIFY(k->waitForLog("Qt Quick UI started successfully", 5000));
        k->stop();
        QCOMPARE(k->exitCode(), 0);
        const QStringList& stderrLines = k->stderrLines();
        const bool noSegfault = std::none_of(stderrLines.cbegin(), stderrLines.cend(), [](const QString& line) {
            return line.contains(QStringLiteral("Segmentation fault"), Qt::CaseInsensitive);
        });
        QVERIFY2(noSegfault, "Stderr must not contain 'Segmentation fault' on exit");
    }
};

QTEST_MAIN(GuiIntegrationTest)
#include "gui_integration_tests.moc"
