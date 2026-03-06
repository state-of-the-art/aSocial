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

#include "Kotik.h"

#include <cstdio>
#include <QDateTime>
#include <QEventLoop>
#include <QTimer>

Kotik::Kotik(QObject* parent)
    : QObject(parent)
    , m_process(new QProcess(this))
{
    Q_ASSERT(m_tmpDir.isValid());

    m_process->setProgram("./aSocial-x86_64.AppImage");
    m_process->setArguments({"--appimage-extract-and-run", "-w", m_tmpDir.path()});

    connect(m_process, &QProcess::finished, this, &Kotik::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &Kotik::onProcessErrorOccurred);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &Kotik::onReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &Kotik::onReadyReadStandardError);

    m_process->start();
}

Kotik::~Kotik()
{
    if( m_process->state() != QProcess::NotRunning ) {
        m_process->write("exit\n");
        if( !m_process->waitForFinished(3000) ) {
            m_process->kill();
            m_process->waitForFinished(1000);
        }
    }
}

void Kotik::write(const QString& command)
{
    m_process->write((command + "\n").toUtf8());
}

bool Kotik::exitApp(int timeoutMs)
{
    m_process->write("exit\n");
    return m_process->waitForFinished(timeoutMs);
}

QString Kotik::tmpPath() const
{
    return m_tmpDir.path();
}

QString Kotik::tmpFilePath(const QString& fileName) const
{
    return m_tmpDir.filePath(fileName);
}

int Kotik::exitCode() const
{
    return m_process->exitCode();
}

void Kotik::onProcessErrorOccurred(QProcess::ProcessError error)
{
    Q_UNUSED(error);
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    fprintf(stderr, "[%s] ------- PROCESS ERROR: %s\n", qPrintable(timestamp), qPrintable(m_process->errorString()));
    fflush(stderr);
}

void Kotik::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus);
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    fprintf(stderr, "[%s] ------- PROCESS FINISHED: %d\n", qPrintable(timestamp), exitCode);
    fflush(stderr);
}

void Kotik::onReadyReadStandardOutput()
{
    processData(m_process->readAllStandardOutput(), m_partialStdout, false);
}

void Kotik::onReadyReadStandardError()
{
    processData(m_process->readAllStandardError(), m_partialStderr, true);
}

void Kotik::processData(const QByteArray& data, QString& partialBuffer, bool isStderr)
{
    if( data.isEmpty() )
        return;

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

    for( QString line : newLines ) {
        if( line.trimmed().isEmpty() )
            continue;

        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");

        m_lines << line;
        if( isStderr )
            m_stderrLines << line;

        fprintf(stderr, "[%s] %s\n", qPrintable(timestamp), qPrintable(line));
        fflush(stderr);

        emit newLogLine(line);
    }

    emit outputUpdated();
}

bool Kotik::contains(const QString& substring, Qt::CaseSensitivity cs) const
{
    if( std::any_of(m_lines.cbegin(), m_lines.cend(), [&](const QString& l) { return l.contains(substring, cs); }) )
        return true;

    return m_partialStdout.contains(substring, cs) || m_partialStderr.contains(substring, cs);
}

bool Kotik::contains(const QRegularExpression& regex) const
{
    if( std::any_of(m_lines.cbegin(), m_lines.cend(), [&](const QString& l) { return regex.match(l).hasMatch(); }) )
        return true;

    return regex.match(m_partialStdout).hasMatch() || regex.match(m_partialStderr).hasMatch();
}

bool Kotik::waitForLog(const QString& substring, int timeoutMs)
{
    if( contains(substring) )
        return true;

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    connect(this, &Kotik::outputUpdated, &loop, [&, substring]() {
        if( contains(substring) )
            loop.quit();
    });

    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(timeoutMs);
    loop.exec();

    return contains(substring);
}

bool Kotik::waitForLog(const QRegularExpression& regex, int timeoutMs)
{
    if( contains(regex) )
        return true;

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    connect(this, &Kotik::outputUpdated, &loop, [&, regex]() {
        if( contains(regex) )
            loop.quit();
    });

    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(timeoutMs);
    loop.exec();

    return contains(regex);
}

bool Kotik::noErrorLogs() const
{
    QRegularExpression errRe(R"(\b(ERROR|CRITICAL|FATAL|Exception)\b)", QRegularExpression::CaseInsensitiveOption);
    return std::none_of(m_stderrLines.cbegin(), m_stderrLines.cend(), [&](const QString& l) {
        return errRe.match(l).hasMatch();
    });
}

void Kotik::clear()
{
    m_lines.clear();
    m_stderrLines.clear();
    m_partialStdout.clear();
    m_partialStderr.clear();
}
