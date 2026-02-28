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

#include "ProcessLogMonitor.h"
#include <cstdio>
#include <QEventLoop>
#include <QTimer>

ProcessLogMonitor::ProcessLogMonitor(QProcess* process, QObject* parent)
    : QObject(parent)
    , m_process(process)
{
    Q_ASSERT(process);

    connect(process, &QProcess::readyReadStandardOutput, this, &ProcessLogMonitor::onReadyReadStandardOutput);
    connect(process, &QProcess::readyReadStandardError, this, &ProcessLogMonitor::onReadyReadStandardError);
}

void ProcessLogMonitor::onReadyReadStandardOutput()
{
    processData(m_process->readAllStandardOutput(), m_partialStdout, false);
}

void ProcessLogMonitor::onReadyReadStandardError()
{
    processData(m_process->readAllStandardError(), m_partialStderr, true);
}

void ProcessLogMonitor::processData(const QByteArray& data, QString& partialBuffer, bool isStderr)
{
    if( data.isEmpty() )
        return;

    // Normalize Windows \r\n and lone \r to \n so lines are clean
    QString chunk = QString::fromUtf8(data);
    chunk.replace("\r\n", "\n").replace('\r', '\n');
    partialBuffer += chunk;

    QStringList newLines = partialBuffer.split('\n', Qt::SkipEmptyParts);

    if( !partialBuffer.endsWith('\n') ) {
        if( !newLines.isEmpty() )
            partialBuffer = newLines.takeLast();
    } else {
        partialBuffer.clear();
    }

    for( QString line : newLines ) { // copy because we may clean it
        if( line.trimmed().isEmpty() )
            continue;

        m_lines << line;
        if( isStderr )
            m_stderrLines << line;

        // === STREAM EVERYTHING to test's stderr in real time ===
        fprintf(stderr, "%s\n", qPrintable(line));
        fflush(stderr);

        emit newLogLine(line);
    }

    // Always notify after any data was processed (complete lines or partial update)
    emit outputUpdated();
}

bool ProcessLogMonitor::contains(const QString& substring, Qt::CaseSensitivity cs) const
{
    // complete lines
    if( std::any_of(m_lines.cbegin(), m_lines.cend(), [&](const QString& l) { return l.contains(substring, cs); }) )
        return true;

    // + partial lines
    return m_partialStdout.contains(substring, cs) || m_partialStderr.contains(substring, cs);
}

bool ProcessLogMonitor::contains(const QRegularExpression& regex) const
{
    // complete lines
    if( std::any_of(m_lines.cbegin(), m_lines.cend(), [&](const QString& l) { return regex.match(l).hasMatch(); }) )
        return true;

    // + partial lines
    return regex.match(m_partialStdout).hasMatch() || regex.match(m_partialStderr).hasMatch();
}

bool ProcessLogMonitor::waitForLog(const QString& substring, int timeoutMs)
{
    if( contains(substring) )
        return true;

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    // This fires on complete lines AND when a partial buffer updates
    connect(this, &ProcessLogMonitor::outputUpdated, &loop, [&, substring]() {
        if( contains(substring) )
            loop.quit();
    });

    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(timeoutMs);
    loop.exec();

    return contains(substring);
}

bool ProcessLogMonitor::waitForLog(const QRegularExpression& regex, int timeoutMs)
{
    if( contains(regex) )
        return true;

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    connect(this, &ProcessLogMonitor::outputUpdated, &loop, [&, regex]() {
        if( contains(regex) )
            loop.quit();
    });

    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(timeoutMs);
    loop.exec();

    return contains(regex);
}

bool ProcessLogMonitor::noErrorLogs() const
{
    QRegularExpression errRe(R"(\b(ERROR|CRITICAL|FATAL|Exception)\b)", QRegularExpression::CaseInsensitiveOption);
    return std::none_of(m_stderrLines.cbegin(), m_stderrLines.cend(), [&](const QString& l) {
        return errRe.match(l).hasMatch();
    });
}

void ProcessLogMonitor::clear()
{
    m_lines.clear();
    m_stderrLines.clear();
    m_partialStdout.clear();
    m_partialStderr.clear();
}
