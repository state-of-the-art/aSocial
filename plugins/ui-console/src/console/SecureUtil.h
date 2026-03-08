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

#ifndef CONSOLE_SECURE_UTIL_H
#define CONSOLE_SECURE_UTIL_H

#include <QByteArray>
#include <QRandomGenerator>
#include <QString>

#include <cstring>

/**
 * @brief Secure-memory utilities for wiping sensitive buffers.
 *
 * Provides helpers that overwrite memory with random data and then zero it
 * before releasing.  This reduces the window during which secrets such as
 * passwords linger in process memory.
 *
 * @note QByteArray (and QString) use implicit sharing (COW).  If the
 *       buffer has been copied by Qt internally, only the current
 *       instance is wiped.  Callers should detach() before writing
 *       sensitive data when possible.
 */
namespace SecureUtil {

/**
 * @brief Overwrite @p buf with random bytes, then zero and clear it.
 *
 * Uses explicit_bzero (Linux/macOS) or a volatile-pointer loop to
 * prevent the compiler from optimising the zeroing away.
 *
 * @param buf  Buffer to wipe; left empty on return.
 */
inline void wipe(QByteArray& buf)
{
    if( buf.isEmpty() )
        return;

    buf.detach();
    char* data = buf.data();
    const int sz = buf.size();

    // Phase 1: overwrite with random data
    for( int i = 0; i < sz; ++i )
        data[i] = static_cast<char>(QRandomGenerator::global()->generate() & 0xFF);

        // Phase 2: zero the buffer in a way the compiler cannot elide
#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    explicit_bzero(data, static_cast<size_t>(sz));
#else
    volatile char* vp = data;
    for( int i = 0; i < sz; ++i )
        vp[i] = 0;
#endif

    buf.clear();
}

/**
 * @brief Overwrite the internal UTF-16 buffer of @p str and clear it.
 * @param str  String to wipe; left empty on return.
 */
inline void wipe(QString& str)
{
    if( str.isEmpty() )
        return;

    str.detach();
    QByteArray utf8 = str.toUtf8();
    wipe(utf8);

    // Wipe the UTF-16 storage directly via QChar*
    QChar* data = str.data();
    const int sz = str.size();
#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    explicit_bzero(data, static_cast<size_t>(sz) * sizeof(QChar));
#else
    volatile QChar* vp = data;
    for( int i = 0; i < sz; ++i )
        vp[i] = QChar(0);
#endif

    str.clear();
}

/**
 * @brief RAII guard that wipes a QByteArray on scope exit.
 *
 * Usage:
 * @code
 *   QByteArray secret = getPassword();
 *   SecureUtil::Guard guard(secret);
 *   // ... use secret ...
 *   // secret is automatically wiped when guard goes out of scope
 * @endcode
 */
class Guard
{
public:
    /** @brief Attach the guard to @p buf (non-owning reference). */
    explicit Guard(QByteArray& buf)
        : m_buf(buf)
    {}

    ~Guard() { wipe(m_buf); }

    Guard(const Guard&) = delete;
    Guard& operator=(const Guard&) = delete;

private:
    QByteArray& m_buf;
};

/**
 * @brief RAII guard that wipes a QString on scope exit.
 */
class StringGuard
{
public:
    /** @brief Attach the guard to @p str (non-owning reference). */
    explicit StringGuard(QString& str)
        : m_str(str)
    {}

    ~StringGuard() { wipe(m_str); }

    StringGuard(const StringGuard&) = delete;
    StringGuard& operator=(const StringGuard&) = delete;

private:
    QString& m_str;
};

} // namespace SecureUtil

#endif // CONSOLE_SECURE_UTIL_H
