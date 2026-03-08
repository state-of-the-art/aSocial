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

#include "Terminal.h"

#ifdef Q_OS_UNIX
#include <poll.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

Terminal::Terminal() = default;

Terminal::~Terminal()
{
    if( m_rawMode )
        disableRawMode();
}

// ---------------------------------------------------------------------------
// Mode management
// ---------------------------------------------------------------------------

void Terminal::enableRawMode()
{
#ifdef Q_OS_UNIX
    if( m_rawMode )
        return;

    ::tcgetattr(STDIN_FILENO, &m_origTermios);

    struct termios raw = m_origTermios;
    raw.c_iflag &= ~static_cast<tcflag_t>(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~static_cast<tcflag_t>(OPOST);
    raw.c_cflag |= CS8;
    raw.c_lflag &= ~static_cast<tcflag_t>(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    ::tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    m_rawMode = true;
#endif
}

void Terminal::disableRawMode()
{
#ifdef Q_OS_UNIX
    if( !m_rawMode )
        return;
    ::tcsetattr(STDIN_FILENO, TCSAFLUSH, &m_origTermios);
    m_rawMode = false;
#endif
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------

bool Terminal::waitForInput(int timeoutMs)
{
#ifdef Q_OS_UNIX
    struct pollfd pfd = {STDIN_FILENO, POLLIN, 0};
    return ::poll(&pfd, 1, timeoutMs) > 0;
#else
    Q_UNUSED(timeoutMs)
    return true;
#endif
}

Terminal::KeyEvent Terminal::readKey()
{
#ifdef Q_OS_UNIX
    char c = 0;
    if( ::read(STDIN_FILENO, &c, 1) != 1 )
        return {Key::Unknown, {}};

    // Escape-sequence handling
    if( c == '\033' ) {
        if( !waitForInput(80) )
            return {Key::Escape, {}};

        char seq[3] = {};
        if( ::read(STDIN_FILENO, &seq[0], 1) != 1 )
            return {Key::Escape, {}};

        if( seq[0] == '[' ) {
            if( !waitForInput(80) )
                return {Key::Escape, {}};
            if( ::read(STDIN_FILENO, &seq[1], 1) != 1 )
                return {Key::Escape, {}};

            if( seq[1] >= '0' && seq[1] <= '9' ) {
                // Extended escape: \033[N~ (Delete, PgUp, etc.)
                if( waitForInput(80) )
                    ::read(STDIN_FILENO, &seq[2], 1); // consume '~'
                if( seq[1] == '3' )
                    return {Key::Delete, {}};
                return {Key::Unknown, {}};
            }

            switch( seq[1] ) {
            case 'A':
                return {Key::Up, {}};
            case 'B':
                return {Key::Down, {}};
            case 'C':
                return {Key::Right, {}};
            case 'D':
                return {Key::Left, {}};
            case 'H':
                return {Key::Home, {}};
            case 'F':
                return {Key::End, {}};
            }
        } else if( seq[0] == 'O' ) {
            if( !waitForInput(80) )
                return {Key::Escape, {}};
            if( ::read(STDIN_FILENO, &seq[1], 1) != 1 )
                return {Key::Escape, {}};
            switch( seq[1] ) {
            case 'H':
                return {Key::Home, {}};
            case 'F':
                return {Key::End, {}};
            }
        }
        return {Key::Escape, {}};
    }

    if( c == '\r' || c == '\n' )
        return {Key::Enter, {}};
    if( c == 127 || c == '\b' )
        return {Key::Backspace, {}};
    if( c == '\t' )
        return {Key::Tab, {}};
    if( c == 3 )
        return {Key::CtrlC, {}};
    if( c == 4 )
        return {Key::CtrlD, {}};
    if( c == 1 )
        return {Key::Home, {}};
    if( c == 5 )
        return {Key::End, {}};
    if( c == 12 ) {
        clearScreen();
        return {Key::Unknown, {}};
    }

    // Multi-byte UTF-8
    if( static_cast<unsigned char>(c) >= 0x80 ) {
        QByteArray bytes;
        bytes.append(c);
        int expected = 0;
        auto uc = static_cast<unsigned char>(c);
        if( (uc & 0xE0) == 0xC0 )
            expected = 1;
        else if( (uc & 0xF0) == 0xE0 )
            expected = 2;
        else if( (uc & 0xF8) == 0xF0 )
            expected = 3;

        for( int i = 0; i < expected; ++i ) {
            char cb = 0;
            if( ::read(STDIN_FILENO, &cb, 1) == 1 )
                bytes.append(cb);
        }
        QString str = QString::fromUtf8(bytes);
        if( !str.isEmpty() )
            return {Key::Char, str.at(0)};
        return {Key::Unknown, {}};
    }

    if( c >= 32 )
        return {Key::Char, QChar(c)};

    return {Key::Unknown, {}};
#else
    return {Key::Unknown, {}};
#endif
}

// ---------------------------------------------------------------------------
// Output
// ---------------------------------------------------------------------------

void Terminal::rawWrite(const char* data, int len)
{
#ifdef Q_OS_UNIX
    ::write(STDOUT_FILENO, data, static_cast<size_t>(len));
#else
    Q_UNUSED(data)
    Q_UNUSED(len)
#endif
}

void Terminal::write(const QString& text)
{
    QByteArray utf8 = text.toUtf8();
    rawWrite(utf8.constData(), utf8.size());
}

void Terminal::writeLine(const QString& text)
{
    write(text);
    rawWrite("\r\n", 2);
}

void Terminal::clearLine()
{
    rawWrite("\033[2K\r", 5);
}

void Terminal::clearToEnd()
{
    rawWrite("\033[K", 3);
}

void Terminal::clearScreen()
{
    rawWrite("\033[2J\033[H", 7);
}

// ---------------------------------------------------------------------------
// Cursor movement
// ---------------------------------------------------------------------------

void Terminal::moveCursorUp(int n)
{
    if( n <= 0 )
        return;
    write(QStringLiteral("\033[%1A").arg(n));
}

void Terminal::moveCursorDown(int n)
{
    if( n <= 0 )
        return;
    write(QStringLiteral("\033[%1B").arg(n));
}

void Terminal::moveCursorForward(int n)
{
    if( n <= 0 )
        return;
    write(QStringLiteral("\033[%1C").arg(n));
}

void Terminal::moveCursorBack(int n)
{
    if( n <= 0 )
        return;
    write(QStringLiteral("\033[%1D").arg(n));
}

void Terminal::moveCursorToColumn(int col)
{
    write(QStringLiteral("\033[%1G").arg(col));
}

void Terminal::hideCursor()
{
    rawWrite("\033[?25l", 6);
}

void Terminal::showCursor()
{
    rawWrite("\033[?25h", 6);
}

// ---------------------------------------------------------------------------
// Terminal geometry
// ---------------------------------------------------------------------------

int Terminal::width() const
{
#ifdef Q_OS_UNIX
    struct winsize ws
    {};
    if( ::ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0 )
        return ws.ws_col;
#endif
    return 80;
}

int Terminal::height() const
{
#ifdef Q_OS_UNIX
    struct winsize ws
    {};
    if( ::ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0 )
        return ws.ws_row;
#endif
    return 24;
}
