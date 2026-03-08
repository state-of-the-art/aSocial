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

#ifndef KOTIK_H
#define KOTIK_H

#include <QObject>
#include <QProcess>
#include <QRegularExpression>
#include <QStringList>
#include <QTemporaryDir>

/**
 * @brief Full lifecycle wrapper for the aSocial CLI process.
 *
 * Creates an isolated temp directory, launches the AppImage with
 * -w pointing there, and provides log monitoring for test assertions.
 */
class Kotik : public QObject
{
    Q_OBJECT

public:
    explicit Kotik(QObject* parent = nullptr);
    ~Kotik();

    void write(const QString& command);

    bool exitApp(int timeoutMs = 3000);

    QString workdirPath() const;
    QString workdirFilePath(const QString& fileName) const;

    int exitCode() const;

    bool contains(const QString& substring, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    bool contains(const QRegularExpression& regex) const;
    bool waitForLog(const QString& substring, int timeoutMs = 10000);
    bool waitForLog(const QRegularExpression& regex, int timeoutMs = 10000);

    bool noErrorLogs() const;

    const QStringList& allLines() const { return m_lines; }
    const QStringList& stderrLines() const { return m_stderrLines; }

    void clear();

signals:
    void newLogLine(const QString& line);
    void outputUpdated();

private slots:
    void onProcessErrorOccurred(QProcess::ProcessError error);
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();

private:
    void processData(const QByteArray& data, QString& partialBuffer, bool isStderr);

    QTemporaryDir m_workdir;
    QProcess* m_process;
    QStringList m_lines;
    QStringList m_stderrLines;
    QString m_partialStdout;
    QString m_partialStderr;
};

#endif // KOTIK_H
