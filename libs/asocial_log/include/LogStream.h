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

#ifndef LOGSTREAM_H
#define LOGSTREAM_H

#include <QDebug>
#include <QString>
#include <QVariant>

#include <type_traits>

#include "LogLevel.h"

class Log;

/**
 * @brief Temporary stream that collects arguments via operator<< and sends
 * the composed message to the current Log on destruction.
 *
 * Supports the same streaming types as QDebug. The buffer is securely
 * wiped (if backed by QByteArray) before destruction when possible.
 * Non-copyable; move constructor clears the source so only one dtor runs.
 */
class LogStream
{
public:
    /** @brief Build a stream for the given log, level, and optional location. */
    LogStream(class Log* log, LogLevel level, const char* file, int line);
    LogStream(LogStream&& other) noexcept;
    LogStream& operator=(LogStream&& other) noexcept;
    ~LogStream();

    LogStream(const LogStream&) = delete;
    LogStream& operator=(const LogStream&) = delete;

    /** @brief Stream QVariant (QDebug with QString backend does not support it). */
    LogStream& operator<<(const QVariant& v)
    {
        if( m_dbg )
            m_buffer += v.toString();
        return *this;
    }

    /** @brief Stream any type supported by QDebug (QVariant excluded; use overload above). */
    template<typename T>
    typename std::enable_if<!std::is_same<T, QVariant>::value, LogStream&>::type operator<<(const T& t)
    {
        if( m_dbg )
            m_dbg->operator<<(t);
        return *this;
    }

    /** @brief Allow manipulators (e.g. Qt::endl). */
    LogStream& operator<<(QDebug (*manipulator)(QDebug))
    {
        if( m_dbg && manipulator )
            manipulator(*m_dbg);
        return *this;
    }

private:
    void flushAndSend();

    class Log* m_log = nullptr;
    LogLevel m_level = LogLevel::Debug;
    const char* m_file = nullptr;
    int m_line = 0;
    QString m_buffer;
    QDebug* m_dbg = nullptr;
    bool m_valid = true;
};

#endif // LOGSTREAM_H
