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

#include "Prompt.h"
#include "Style.h"
#include "Terminal.h"

Prompt::Prompt(Terminal& term)
    : m_term(term)
{}

// ===========================================================================
// askText
// ===========================================================================

QString Prompt::askText(const QString& question, const QString& defaultValue)
{
    QString hint;
    if( !defaultValue.isEmpty() )
        hint = QStringLiteral(" %1[%2]%3").arg(Style::muted(), defaultValue, Style::reset());

    m_term.write(QStringLiteral("  %1%2?%3 %4%5: ").arg(Style::info(), Style::bold(), Style::reset(), question, hint));

    // Simple line-reading loop (no history needed for prompts)
    QString buffer;
    int cursor = 0;

    for( ;; ) {
        Terminal::KeyEvent ev = m_term.readKey();

        switch( ev.key ) {
        case Terminal::Key::Char:
            buffer.insert(cursor, ev.ch);
            ++cursor;
            // Redraw from cursor position
            m_term.clearToEnd();
            m_term.write(buffer.mid(cursor - 1));
            if( cursor < buffer.size() )
                m_term.moveCursorBack(buffer.size() - cursor);
            break;

        case Terminal::Key::Backspace:
            if( cursor > 0 ) {
                buffer.remove(cursor - 1, 1);
                --cursor;
                m_term.moveCursorBack(1);
                m_term.clearToEnd();
                m_term.write(buffer.mid(cursor));
                if( cursor < buffer.size() )
                    m_term.moveCursorBack(buffer.size() - cursor);
            }
            break;

        case Terminal::Key::Left:
            if( cursor > 0 ) {
                --cursor;
                m_term.moveCursorBack(1);
            }
            break;

        case Terminal::Key::Right:
            if( cursor < buffer.size() ) {
                ++cursor;
                m_term.moveCursorForward(1);
            }
            break;

        case Terminal::Key::Enter:
            m_term.writeLine();
            if( buffer.isEmpty() && !defaultValue.isEmpty() )
                return defaultValue;
            return buffer;

        case Terminal::Key::CtrlC:
            m_term.writeLine();
            return {};

        default:
            break;
        }
    }
}

// ===========================================================================
// askPassword
// ===========================================================================

QString Prompt::askPassword(const QString& question)
{
    m_term.write(QStringLiteral("  %1%2?%3 %4: ").arg(Style::info(), Style::bold(), Style::reset(), question));

    QString buffer;

    for( ;; ) {
        Terminal::KeyEvent ev = m_term.readKey();

        switch( ev.key ) {
        case Terminal::Key::Char:
            buffer.append(ev.ch);
            m_term.write(QStringLiteral("*"));
            break;

        case Terminal::Key::Backspace:
            if( !buffer.isEmpty() ) {
                buffer.chop(1);
                m_term.moveCursorBack(1);
                m_term.clearToEnd();
            }
            break;

        case Terminal::Key::Enter:
            // Cleaning the input to not leave the password size in history
            m_term.moveCursorBack(buffer.size());
            m_term.clearToEnd();

            m_term.writeLine();
            return buffer;

        case Terminal::Key::CtrlC:
            // Cleaning the input to not leave the password size in history
            m_term.moveCursorBack(buffer.size());
            m_term.clearToEnd();

            m_term.writeLine();
            return {};

        default:
            break;
        }
    }
}

// ===========================================================================
// askSelect
// ===========================================================================

int Prompt::askSelect(const QString& question, const QStringList& options, int defaultIndex)
{
    if( options.isEmpty() )
        return -1;

    int selected = qBound(0, defaultIndex, options.size() - 1);

    m_term.writeLine(QStringLiteral("  %1%2?%3 %4").arg(Style::info(), Style::bold(), Style::reset(), question));

    // Render helper
    auto render = [&]() {
        // Move cursor up to the first option line to redraw in-place
        for( int i = 0; i < options.size(); ++i ) {
            m_term.clearLine();
            if( i == selected ) {
                m_term.writeLine(QStringLiteral("    %1%2 %3%4%5")
                                     .arg(Style::accent(), Style::pointer, Style::bold(), options[i], Style::reset()));
            } else {
                m_term.writeLine(QStringLiteral("      %1%2%3").arg(Style::muted(), options[i], Style::reset()));
            }
        }
        // Move back up so next redraw overwrites the same lines
        m_term.moveCursorUp(options.size());
    };

    m_term.hideCursor();
    render();

    for( ;; ) {
        Terminal::KeyEvent ev = m_term.readKey();

        switch( ev.key ) {
        case Terminal::Key::Up:
            if( selected > 0 ) {
                --selected;
                render();
            }
            break;

        case Terminal::Key::Down:
            if( selected < options.size() - 1 ) {
                ++selected;
                render();
            }
            break;

        case Terminal::Key::Enter:
            // Move past the options and show the selection
            m_term.moveCursorDown(options.size());
            m_term.showCursor();
            m_term.writeLine(QStringLiteral("  %1%2%3 %4")
                                 .arg(Style::success(), Style::checkmark, Style::reset(), options[selected]));
            return selected;

        case Terminal::Key::CtrlC:
            m_term.moveCursorDown(options.size());
            m_term.showCursor();
            m_term.writeLine();
            return -1;

        default:
            break;
        }
    }
}

// ===========================================================================
// askConfirm
// ===========================================================================

bool Prompt::askConfirm(const QString& question, bool defaultYes)
{
    QString hint = defaultYes ? QStringLiteral("Y/n") : QStringLiteral("y/N");

    m_term.write(QStringLiteral("  %1%2?%3 %4 %5(%6)%7: ")
                     .arg(Style::info(), Style::bold(), Style::reset(), question, Style::muted(), hint, Style::reset()));

    for( ;; ) {
        Terminal::KeyEvent ev = m_term.readKey();

        switch( ev.key ) {
        case Terminal::Key::Char:
            if( ev.ch == QLatin1Char('y') || ev.ch == QLatin1Char('Y') ) {
                m_term.writeLine(QStringLiteral("yes"));
                return true;
            }
            if( ev.ch == QLatin1Char('n') || ev.ch == QLatin1Char('N') ) {
                m_term.writeLine(QStringLiteral("no"));
                return false;
            }
            break;

        case Terminal::Key::Enter:
            m_term.writeLine(defaultYes ? QStringLiteral("yes") : QStringLiteral("no"));
            return defaultYes;

        case Terminal::Key::CtrlC:
            m_term.writeLine();
            return false;

        default:
            break;
        }
    }
}

// ===========================================================================
// askNumber
// ===========================================================================

int Prompt::askNumber(const QString& question, int min, int max, int defaultValue)
{
    for( ;; ) {
        QString hint = QStringLiteral("%1-%2").arg(min).arg(max);
        if( defaultValue >= min && defaultValue <= max )
            hint += QStringLiteral(", default %1").arg(defaultValue);

        QString answer = askText(QStringLiteral("%1 (%2)").arg(question, hint));

        if( answer.isNull() )
            return defaultValue;

        if( answer.isEmpty() && defaultValue >= min && defaultValue <= max )
            return defaultValue;

        bool ok = false;
        int val = answer.toInt(&ok);
        if( ok && val >= min && val <= max )
            return val;

        m_term.writeLine(QStringLiteral("  %1Please enter a number between %2 and %3.%4")
                             .arg(Style::warning())
                             .arg(min)
                             .arg(max)
                             .arg(Style::reset()));
    }
}
