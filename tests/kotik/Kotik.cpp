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

#include <algorithm>
#include <cstdio>
#include <QDateTime>
#include <QEventLoop>
#include <QRegularExpression>
#include <QTextStream>
#include <QTimer>

// Strip ANSI escape sequences (colors, cursor movement, etc.) from a log line
static QString stripAnsiSequences(const QString& input)
{
    // Matches ESC [ ... command (CSI sequences)
    static const QRegularExpression ansiRe(QStringLiteral(R"(\x1B\[[0-9;?]*[ -/]*[@-~])"));
    return QString(input).remove(ansiRe);
}

Kotik::Kotik(QObject* parent, bool start)
    : QObject(parent)
    , m_process(new QProcess(this))
{
    Q_ASSERT(m_workdir.isValid());

    m_process->setProgram("./aSocial-x86_64.AppImage");

    connect(m_process, &QProcess::finished, this, &Kotik::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &Kotik::onProcessErrorOccurred);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &Kotik::onReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &Kotik::onReadyReadStandardError);

    if( start )
        this->start();
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

void Kotik::setWorkdir(const QString workdir)
{
    m_workdir_path = workdir;
}

void Kotik::setEnv(const QList<QPair<QString, QString>> newEnv)
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    for( auto item : newEnv ) {
        env.insert(item.first, item.second);
    }
    m_process->setProcessEnvironment(env);
}

void Kotik::start()
{
    this->start({"--no-gui"});
}

void Kotik::start(const QStringList& arguments)
{
    QStringList args = {"--appimage-extract-and-run", "-w", workdirPath()};
    args.append(arguments);
    m_process->setArguments(args);
    m_process->start();
    m_process_id = m_process->processId();
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    fprintf(stderr, "[%s] ------- PROCESS %lld STARTED\n", qPrintable(timestamp), m_process_id);
}

void Kotik::stop() const
{
    m_process->terminate();
}

void Kotik::kill() const
{
    m_process->kill();
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

QString Kotik::workdirPath() const
{
    if( m_workdir_path.isEmpty() )
        return m_workdir.path();
    return m_workdir_path;
}

QString Kotik::workdirFilePath(const QString& fileName) const
{
    if( m_workdir_path.isEmpty() )
        return m_workdir.filePath(fileName);
    return QDir(m_workdir_path).filePath(fileName);
}

int Kotik::exitCode() const
{
    return m_process->exitCode();
}

void Kotik::onProcessErrorOccurred(QProcess::ProcessError error)
{
    Q_UNUSED(error);
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    fprintf(
        stderr,
        "[%s] ------- PROCESS %lld ERROR: %s\n",
        qPrintable(timestamp),
        m_process_id,
        qPrintable(m_process->errorString()));
    fflush(stderr);
}

void Kotik::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus);
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    fprintf(stderr, "[%s] ------- PROCESS %lld FINISHED: %d\n", qPrintable(timestamp), m_process_id, exitCode);
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
    chunk = stripAnsiSequences(chunk);
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
        line = stripAnsiSequences(line);
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

bool Kotik::findValuesImpl(const QString& pattern, const QVector<QString*>& outs)
{
    if( std::any_of(outs.cbegin(), outs.cend(), [](QString* p) { return p == nullptr; }) )
        return false;

    // Split pattern by literal "%s" tokens into required parts around the captures.
    // Example: "Bob (id=%s)" -> ["Bob (id=", ")"] with 1 capture.
    QStringList parts;
    QString current;
    int i = 0;
    while( i < pattern.size() ) {
        if( i + 1 < pattern.size() && pattern[i] == '%' && pattern[i + 1] == 's' ) {
            parts << current;
            current.clear();
            i += 2;
        } else {
            current.append(pattern[i]);
            i++;
        }
    }
    parts << current;

    const int expectedCaptures = parts.size() - 1;
    if( expectedCaptures != outs.size() )
        return false;

    auto extractFromText = [&](const QString& text, QVector<QString>& capturedOut) -> bool {
        if( text.isNull() )
            return false;

        capturedOut.fill(QString(), outs.size());

        const QString& prefix = parts.front();
        if( prefix.isEmpty() ) {
            int cur = 0;
            for( int idx = 0; idx < outs.size(); idx++ ) {
                const QString& nextLiteral = parts[idx + 1];
                const int nextPos = text.indexOf(nextLiteral, cur);
                if( nextPos < 0 )
                    return false;
                capturedOut[idx] = text.mid(cur, nextPos - cur);
                cur = nextPos + nextLiteral.size();
            }
            return true;
        }

        int startPos = text.indexOf(prefix, 0);
        while( startPos >= 0 ) {
            int cur = startPos + prefix.size();
            bool ok = true;
            for( int idx = 0; idx < outs.size(); idx++ ) {
                const QString& nextLiteral = parts[idx + 1];
                const int nextPos = text.indexOf(nextLiteral, cur);
                if( nextPos < 0 ) {
                    ok = false;
                    break;
                }
                capturedOut[idx] = text.mid(cur, nextPos - cur);
                cur = nextPos + nextLiteral.size();
            }
            if( ok )
                return true;

            startPos = text.indexOf(prefix, startPos + 1);
        }
        return false;
    };

    auto tryScanNow = [&]() -> bool {
        for( const QString& line : m_lines ) {
            QVector<QString> captured;
            if( extractFromText(line, captured) ) {
                for( int idx = 0; idx < outs.size(); idx++ )
                    *outs[idx] = captured[idx];
                return true;
            }
        }

        QVector<QString> captured;
        if( extractFromText(m_partialStdout, captured) ) {
            for( int idx = 0; idx < outs.size(); idx++ )
                *outs[idx] = captured[idx];
            return true;
        }

        captured.clear();
        if( extractFromText(m_partialStderr, captured) ) {
            for( int idx = 0; idx < outs.size(); idx++ )
                *outs[idx] = captured[idx];
            return true;
        }

        return false;
    };

    return tryScanNow();
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
