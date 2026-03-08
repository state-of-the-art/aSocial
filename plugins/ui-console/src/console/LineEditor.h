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

#ifndef CONSOLE_LINE_EDITOR_H
#define CONSOLE_LINE_EDITOR_H

#include <QStringList>

class Terminal;

/**
 * @brief Interactive line editor with history and tab completion.
 *
 * Provides GNU-readline-style editing using the raw Terminal backend:
 * cursor movement (Left/Right, Home/End), character insert/delete,
 * command history (Up/Down), and prefix-based tab completion.
 *
 * The editor blocks on readLine() until the user presses Enter or
 * signals EOF (Ctrl-D on an empty line).
 */
class LineEditor
{
public:
    /**
     * @brief Construct an editor bound to the given terminal.
     * @param term  Raw terminal used for key reads and output (non-owning).
     */
    explicit LineEditor(Terminal& term);

    /**
     * @brief Read a complete line of input from the user.
     *
     * Displays @p prompt, then enters a key-event loop that supports
     * full line-editing until Enter is pressed.
     *
     * @param prompt       Text shown before the cursor (may contain ANSI).
     * @param completions  Candidate strings for tab completion.
     * @return The entered line, or a null QString on EOF (Ctrl-D).
     */
    QString readLine(const QString& prompt, const QStringList& completions = {});

    /** @brief Append a line to the history ring (duplicates are suppressed). */
    void addToHistory(const QString& line);

    /** @brief Replace the entire history. */
    void setHistory(const QStringList& history);

    /** @brief Return a copy of the current history entries. */
    QStringList history() const { return m_history; }

    /** @brief Maximum number of history entries kept (default 200). */
    void setMaxHistory(int n) { m_maxHistory = n; }

private:
    void redrawLine(const QString& prompt);
    void handleTab(const QStringList& completions);

    /** @brief Compute the visible (non-ANSI) length of a string. */
    static int visibleLength(const QString& s);

    Terminal& m_term;
    QStringList m_history;
    int m_maxHistory = 200;

    // Per-readLine state
    QString m_buffer;
    int m_cursor = 0;
    int m_historyIdx = -1;
    QString m_savedBuffer;
};

#endif // CONSOLE_LINE_EDITOR_H
