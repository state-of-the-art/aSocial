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

#ifndef CONSOLE_TERMINAL_H
#define CONSOLE_TERMINAL_H

#include <QString>

#ifdef Q_OS_UNIX
#include <termios.h>
#endif

/**
 * @brief Low-level terminal I/O with raw-mode management and ANSI helpers.
 *
 * Wraps POSIX termios to switch between cooked and raw modes, reads
 * individual key events (including multi-byte escape sequences for
 * arrow keys, Home/End, Delete), and provides convenience methods for
 * ANSI cursor/line manipulation.
 *
 * All output goes through write() which converts QString to UTF-8 and
 * writes directly to STDOUT_FILENO, bypassing stdio buffering for
 * immediate terminal updates.
 */
class Terminal
{
public:
    /** @brief Key identifiers returned by readKey(). */
    enum class Key {
        Char, ///< Printable character (see KeyEvent::ch)
        Up,
        Down,
        Left,
        Right,
        Home,
        End,
        Delete,
        Backspace,
        Tab,
        Enter,
        Escape,
        CtrlC,
        CtrlD,
        Unknown
    };

    /** @brief A single key-press event. */
    struct KeyEvent
    {
        Key key = Key::Unknown;
        QChar ch; ///< Valid only when key == Key::Char
    };

    Terminal();
    ~Terminal();

    Terminal(const Terminal&) = delete;
    Terminal& operator=(const Terminal&) = delete;

    // --- Mode management ---------------------------------------------------

    /** @brief Switch the terminal to raw mode (no echo, char-by-char). */
    void enableRawMode();

    /** @brief Restore the terminal to its original mode. */
    void disableRawMode();

    /** @brief true if the terminal is currently in raw mode. */
    bool isRawMode() const { return m_rawMode; }

    // --- Input -------------------------------------------------------------

    /**
     * @brief Block until a key event is available and return it.
     *
     * Handles multi-byte escape sequences (arrow keys, Home, End, Delete)
     * and basic UTF-8 decoding for printable characters.
     */
    KeyEvent readKey();

    // --- Output ------------------------------------------------------------

    /** @brief Write UTF-8 text directly to stdout. */
    void write(const QString& text);

    /** @brief Write text followed by a newline. */
    void writeLine(const QString& text = {});

    /** @brief Erase the entire current line and move cursor to column 1. */
    void clearLine();

    /** @brief Erase from the cursor to the end of the line. */
    void clearToEnd();

    /** @brief Clear the entire screen and move cursor to (1,1). */
    void clearScreen();

    // --- Cursor movement ---------------------------------------------------

    /** @brief Move cursor up by @p n rows. */
    void moveCursorUp(int n = 1);

    /** @brief Move cursor down by @p n rows. */
    void moveCursorDown(int n = 1);

    /** @brief Move cursor right by @p n columns. */
    void moveCursorForward(int n = 1);

    /** @brief Move cursor left by @p n columns. */
    void moveCursorBack(int n = 1);

    /** @brief Move cursor to column @p col (1-based). */
    void moveCursorToColumn(int col);

    /** @brief Hide the blinking cursor. */
    void hideCursor();

    /** @brief Show the blinking cursor. */
    void showCursor();

    // --- Terminal geometry --------------------------------------------------

    /** @brief Current terminal width in columns. */
    int width() const;

    /** @brief Current terminal height in rows. */
    int height() const;

private:
    /** @brief Check if more input bytes are available within @p timeoutMs. */
    bool waitForInput(int timeoutMs);

    /** @brief Write a raw byte string to stdout. */
    void rawWrite(const char* data, int len);

#ifdef Q_OS_UNIX
    struct termios m_origTermios
    {};
#endif
    bool m_rawMode = false;
};

#endif // CONSOLE_TERMINAL_H
