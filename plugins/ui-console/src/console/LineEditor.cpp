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

#include "LineEditor.h"
#include "Terminal.h"

#include <QRegularExpression>

LineEditor::LineEditor(Terminal& term)
    : m_term(term)
{}

// ---------------------------------------------------------------------------
// History
// ---------------------------------------------------------------------------

void LineEditor::addToHistory(const QString& line)
{
    if( line.trimmed().isEmpty() )
        return;
    if( !m_history.isEmpty() && m_history.last() == line )
        return;
    m_history.append(line);
    while( m_history.size() > m_maxHistory )
        m_history.removeFirst();
}

void LineEditor::setHistory(const QStringList& history)
{
    m_history = history;
    while( m_history.size() > m_maxHistory )
        m_history.removeFirst();
}

// ---------------------------------------------------------------------------
// Visible (non-ANSI) length
// ---------------------------------------------------------------------------

int LineEditor::visibleLength(const QString& s)
{
    static const QRegularExpression ansi(QStringLiteral("\033\\[[0-9;]*[a-zA-Z]"));
    QString clean = s;
    clean.remove(ansi);
    return clean.size();
}

// ---------------------------------------------------------------------------
// Redraw
// ---------------------------------------------------------------------------

void LineEditor::redrawLine(const QString& prompt)
{
    m_term.clearLine();
    m_term.write(prompt + m_buffer);
    // Position the cursor correctly
    int tailLen = m_buffer.size() - m_cursor;
    if( tailLen > 0 )
        m_term.moveCursorBack(tailLen);
}

// ---------------------------------------------------------------------------
// Tab completion
// ---------------------------------------------------------------------------

void LineEditor::handleTab(const QStringList& completions)
{
    if( completions.isEmpty() )
        return;

    // Find candidates that match the current buffer as a prefix
    QStringList matches;
    for( const QString& c : completions ) {
        if( c.startsWith(m_buffer, Qt::CaseInsensitive) )
            matches.append(c);
    }

    if( matches.isEmpty() )
        return;

    if( matches.size() == 1 ) {
        m_buffer = matches.first() + QLatin1Char(' ');
        m_cursor = m_buffer.size();
        return;
    }

    // Find longest common prefix among matches
    QString common = matches.first();
    for( int i = 1; i < matches.size(); ++i ) {
        int len = qMin(common.size(), matches[i].size());
        int j = 0;
        while( j < len && common[j].toLower() == matches[i][j].toLower() )
            ++j;
        common = common.left(j);
    }
    if( common.size() > m_buffer.size() ) {
        m_buffer = common;
        m_cursor = m_buffer.size();
    } else {
        // Show all candidates below the prompt
        m_term.writeLine();
        for( const QString& m : matches )
            m_term.writeLine(QStringLiteral("  ") + m);
    }
}

// ---------------------------------------------------------------------------
// Main read loop
// ---------------------------------------------------------------------------

QString LineEditor::readLine(const QString& prompt, const QStringList& completions)
{
    m_buffer.clear();
    m_cursor = 0;
    m_historyIdx = -1;
    m_savedBuffer.clear();

    m_term.write(prompt);

    for( ;; ) {
        Terminal::KeyEvent ev = m_term.readKey();

        switch( ev.key ) {
        case Terminal::Key::Char:
            m_buffer.insert(m_cursor, ev.ch);
            ++m_cursor;
            redrawLine(prompt);
            break;

        case Terminal::Key::Backspace:
            if( m_cursor > 0 ) {
                m_buffer.remove(m_cursor - 1, 1);
                --m_cursor;
                redrawLine(prompt);
            }
            break;

        case Terminal::Key::Delete:
            if( m_cursor < m_buffer.size() ) {
                m_buffer.remove(m_cursor, 1);
                redrawLine(prompt);
            }
            break;

        case Terminal::Key::Left:
            if( m_cursor > 0 ) {
                --m_cursor;
                m_term.moveCursorBack(1);
            }
            break;

        case Terminal::Key::Right:
            if( m_cursor < m_buffer.size() ) {
                ++m_cursor;
                m_term.moveCursorForward(1);
            }
            break;

        case Terminal::Key::Home:
            if( m_cursor > 0 ) {
                m_term.moveCursorBack(m_cursor);
                m_cursor = 0;
            }
            break;

        case Terminal::Key::End:
            if( m_cursor < m_buffer.size() ) {
                m_term.moveCursorForward(m_buffer.size() - m_cursor);
                m_cursor = m_buffer.size();
            }
            break;

        case Terminal::Key::Up:
            if( !m_history.isEmpty() ) {
                if( m_historyIdx < 0 ) {
                    m_savedBuffer = m_buffer;
                    m_historyIdx = m_history.size() - 1;
                } else if( m_historyIdx > 0 ) {
                    --m_historyIdx;
                }
                m_buffer = m_history[m_historyIdx];
                m_cursor = m_buffer.size();
                redrawLine(prompt);
            }
            break;

        case Terminal::Key::Down:
            if( m_historyIdx >= 0 ) {
                if( m_historyIdx < m_history.size() - 1 ) {
                    ++m_historyIdx;
                    m_buffer = m_history[m_historyIdx];
                } else {
                    m_historyIdx = -1;
                    m_buffer = m_savedBuffer;
                }
                m_cursor = m_buffer.size();
                redrawLine(prompt);
            }
            break;

        case Terminal::Key::Tab:
            handleTab(completions);
            redrawLine(prompt);
            break;

        case Terminal::Key::Enter:
            m_term.writeLine();
            return m_buffer;

        case Terminal::Key::CtrlC:
            m_buffer.clear();
            m_cursor = 0;
            m_term.writeLine();
            return QString();

        case Terminal::Key::CtrlD:
            if( m_buffer.isEmpty() ) {
                m_term.writeLine();
                return QString("exit");
            }
            // Non-empty line: treat as Delete
            if( m_cursor < m_buffer.size() ) {
                m_buffer.remove(m_cursor, 1);
                redrawLine(prompt);
            }
            break;

        default:
            break;
        }
    }
}
