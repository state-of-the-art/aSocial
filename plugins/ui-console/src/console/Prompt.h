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

#ifndef CONSOLE_PROMPT_H
#define CONSOLE_PROMPT_H

#include <QString>
#include <QStringList>

class Terminal;

/**
 * @brief Rich interactive prompts for the console UI.
 *
 * Provides several input widgets that temporarily take over the Terminal
 * to collect user input in different styles:
 *
 * - **askText**     – free-form text with an optional default value.
 * - **askPassword** – masked input (shows * per character).
 * - **askSelect**   – arrow-navigable list of choices.
 * - **askConfirm**  – simple yes/no question.
 * - **askNumber**   – integer input with range validation.
 *
 * Every method blocks until the user confirms or cancels; cancellation
 * (Ctrl-C) is indicated by a null/empty return or -1 index.
 */
class Prompt
{
public:
    /**
     * @brief Construct a Prompt bound to the given terminal.
     * @param term  Terminal for input/output (non-owning).
     */
    explicit Prompt(Terminal& term);

    /**
     * @brief Ask for free-form text input.
     *
     * @param question      Styled question text (shown with a "?" prefix).
     * @param defaultValue  Pre-filled value; returned if the user presses
     *                      Enter without typing.
     * @return The entered text, or @p defaultValue on empty input.
     *         Null QString on Ctrl-C.
     */
    QString askText(const QString& question, const QString& defaultValue = {});

    /**
     * @brief Ask for a password (characters shown as '*').
     *
     * The caller is responsible for secure-wiping the returned string.
     *
     * @param question  Styled question text.
     * @return The entered password.  Null QString on Ctrl-C.
     */
    QString askPassword(const QString& question);

    /**
     * @brief Present a selectable list and return the chosen index.
     *
     * The user navigates with Up/Down arrows and confirms with Enter.
     * The list scrolls when it exceeds the visible terminal height.
     *
     * @param question      Question shown above the list.
     * @param options       Choices to display.
     * @param defaultIndex  Initially highlighted item (0-based).
     * @return Selected index (0-based), or -1 on Ctrl-C.
     */
    int askSelect(const QString& question, const QStringList& options, int defaultIndex = 0);

    /**
     * @brief Ask a yes/no question.
     *
     * @param question    Question text.
     * @param defaultYes  true → "(Y/n)", false → "(y/N)".
     * @return true for yes, false for no.
     */
    bool askConfirm(const QString& question, bool defaultYes = false);

    /**
     * @brief Ask for an integer within a range.
     *
     * Re-prompts on invalid or out-of-range input.
     *
     * @param question      Question text.
     * @param min           Minimum accepted value.
     * @param max           Maximum accepted value.
     * @param defaultValue  Value returned on empty input.
     * @return The entered integer, or @p defaultValue on empty input.
     *         Returns @p defaultValue on Ctrl-C.
     */
    int askNumber(const QString& question, int min, int max, int defaultValue = 0);

private:
    Terminal& m_term;
};

#endif // CONSOLE_PROMPT_H
