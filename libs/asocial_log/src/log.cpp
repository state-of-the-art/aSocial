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

#include "Log.h"
#include "plugin/UiPluginInterface.h"

#include <QRandomGenerator>

#include <cstring>

namespace {
/** @brief Overwrite @p buf with random bytes, then clear (secure wipe). */
void secureWipe(QByteArray& buf)
{
    if( buf.isEmpty() )
        return;
    auto* rng = QRandomGenerator::global();
    auto* data = reinterpret_cast<quint32*>(buf.data());
    const qsizetype words = buf.size() / static_cast<qsizetype>(sizeof(quint32));
    for( qsizetype i = 0; i < words; ++i )
        data[i] = rng->generate();
    const qsizetype tail = buf.size() % static_cast<qsizetype>(sizeof(quint32));
    for( qsizetype i = buf.size() - tail; i < buf.size(); ++i )
        buf[i] = static_cast<char>(rng->generate() & 0xFF);
    buf.clear();
}
} // namespace

// ---------------------------------------------------------------------------
// LogStream
// ---------------------------------------------------------------------------

LogStream::LogStream(Log* log, LogLevel level, const char* file, int line)
    : m_log(log)
    , m_level(level)
    , m_file(file)
    , m_line(line)
    , m_dbg(new QDebug(&m_buffer))
{}

LogStream::LogStream(LogStream&& other) noexcept
    : m_log(other.m_log)
    , m_level(other.m_level)
    , m_file(other.m_file)
    , m_line(other.m_line)
    , m_buffer(std::move(other.m_buffer))
    , m_dbg(other.m_dbg)
    , m_valid(other.m_valid)
{
    other.m_dbg = nullptr;
    other.m_valid = false;
}

LogStream& LogStream::operator=(LogStream&& other) noexcept
{
    if( this != &other ) {
        if( m_dbg )
            delete m_dbg;
        m_log = other.m_log;
        m_level = other.m_level;
        m_file = other.m_file;
        m_line = other.m_line;
        m_buffer = std::move(other.m_buffer);
        m_dbg = other.m_dbg;
        m_valid = other.m_valid;
        other.m_dbg = nullptr;
        other.m_valid = false;
    }
    return *this;
}

LogStream::~LogStream()
{
    if( m_valid && m_log && m_dbg ) {
        delete m_dbg;
        m_dbg = nullptr;
        flushAndSend();
    } else if( m_dbg ) {
        delete m_dbg;
        m_dbg = nullptr;
    }
}

void LogStream::flushAndSend()
{
    if( !m_log )
        return;
    QString msg = m_buffer;
    QByteArray utf8 = msg.toUtf8();
    m_log->output(m_level, msg, m_file, m_line);
    secureWipe(utf8);
}

// ---------------------------------------------------------------------------
// Log
// ---------------------------------------------------------------------------

Log::Log(const QString& name, LogLevel verbosity, Log* parent)
    : m_name(name)
    , m_verbosity(verbosity)
    , m_parent(parent)
{}

Log::~Log() = default;

void Log::setSink(UiPluginInterface* sink)
{
    Log* r = root();
    if( r )
        r->m_sink = sink;
}

Log* Log::root()
{
    Log* cur = this;
    while( cur && cur->m_parent )
        cur = cur->m_parent;
    return cur;
}

void Log::output(LogLevel level, const QString& msg, const char* file, int line)
{
    QString formatted = m_name.isEmpty() ? msg : QStringLiteral("[%1] %2").arg(m_name, msg);
#if defined(QT_DEBUG) || !defined(NDEBUG)
    if( file && line > 0 ) {
        const char* base = std::strrchr(file, '/');
        if( base )
            file = base + 1;
        else {
            base = std::strrchr(file, '\\');
            if( base )
                file = base + 1;
        }
        formatted += QStringLiteral(" (%1:%2)").arg(QString::fromUtf8(file)).arg(line);
    }
#endif
    if( m_parent )
        m_parent->output(level, formatted, nullptr, 0);
    else if( m_sink )
        m_sink->logSink(level, formatted);
    else {
        switch( level ) {
        case LogLevel::Debug:
            qDebug().noquote() << formatted;
            break;
        case LogLevel::Info:
            qInfo().noquote() << formatted;
            break;
        case LogLevel::Warning:
            qWarning().noquote() << formatted;
            break;
        case LogLevel::Critical:
            qCritical().noquote() << formatted;
            break;
        case LogLevel::Fatal:
            qFatal("%s", qPrintable(formatted));
            break;
        }
    }
}

LogStream Log::d(const char* file, int line)
{
    return LogStream(this, LogLevel::Debug, file, line);
}

LogStream Log::i(const char* file, int line)
{
    return LogStream(this, LogLevel::Info, file, line);
}

LogStream Log::w(const char* file, int line)
{
    return LogStream(this, LogLevel::Warning, file, line);
}

LogStream Log::c(const char* file, int line)
{
    return LogStream(this, LogLevel::Critical, file, line);
}

LogStream Log::f(const char* file, int line)
{
    return LogStream(this, LogLevel::Fatal, file, line);
}
