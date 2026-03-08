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

#ifndef CONSOLEUI_H
#define CONSOLEUI_H

#include "CoreInterface.h"

#include <memory>
#include <QThread>

class CommandRouter;
class LineEditor;
class Prompt;
class Spinner;
class Terminal;

/**
 * @brief Worker thread that drives the interactive console session.
 *
 * Creates the full console framework stack (Terminal, LineEditor, Prompt,
 * Spinner, CommandRouter) in its own thread and enters a read-eval-print
 * loop until the user types "exit" or signals EOF.
 *
 * All data access flows through CoreInterface CRUD methods — no direct
 * database calls from the UI layer.
 */
class ConsoleUI : public QThread
{
    Q_OBJECT

public:
    ConsoleUI();
    ~ConsoleUI() override;

    /** @brief Register all commands and build the command tree. */
    void configure();

    /** @brief Inject the Core access proxy. */
    void setCore(CoreInterface* core) { m_core = core; }

private:
    void run() override;

    // --- Welcome banner ----------------------------------------------------
    void showBanner();

    // --- Command builders --------------------------------------------------
    void buildSettingsCommands();
    void buildInitCommands();
    void buildProfileCommands();
    void buildPersonaCommands();
    void buildContactCommands();
    void buildGroupCommands();
    void buildMessageCommands();
    void buildEventCommands();
    void buildUtilityCommands();

    // --- Prompt helpers ----------------------------------------------------
    QString promptText() const;

    CoreInterface* m_core = nullptr;

    // Framework components (created on the worker thread in run())
    std::unique_ptr<Terminal> m_term;
    std::unique_ptr<LineEditor> m_editor;
    std::unique_ptr<Prompt> m_prompt;
    std::unique_ptr<Spinner> m_spinner;
    std::unique_ptr<CommandRouter> m_router;
};

#endif // CONSOLEUI_H
