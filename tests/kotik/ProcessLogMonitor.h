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

#ifndef PROCESSLOGMONITOR_H
#define PROCESSLOGMONITOR_H

#include <QObject>
#include <QProcess>
#include <QRegularExpression>
#include <QStringList>

class ProcessLogMonitor : public QObject
{
    Q_OBJECT
public:
    explicit ProcessLogMonitor(QProcess* process, QObject* parent = nullptr);

    // Validation on ALL output (stdout + stderr)
    bool contains(const QString& substring, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    bool contains(const QRegularExpression& regex) const;
    bool waitForLog(const QString& substring, int timeoutMs = 10000);
    bool waitForLog(const QRegularExpression& regex, int timeoutMs = 10000);

    // Error check – looks only at stderr (logs), ignores normal stdout responses
    bool noErrorLogs() const;

    const QStringList& allLines() const { return m_lines; }
    const QStringList& stderrLines() const { return m_stderrLines; } // if you ever need it

    void clear();

signals:
    void newLogLine(const QString& line);
    void outputUpdated();

private slots:
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();

private:
    void processData(const QByteArray& data, QString& partialBuffer, bool isStderr);

    QProcess* m_process = nullptr;
    QStringList m_lines;       // combined for easy validation
    QStringList m_stderrLines; // only stderr for noErrorLogs
    QString m_partialStdout;
    QString m_partialStderr;
};

#endif // PROCESSLOGMONITOR_H
