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

#ifndef UIWORKER_H
#define UIWORKER_H

#include <QThread>

#include "CoreInterface.h"
#include "cli/cli.h"

/**
 * @brief Custom CLI menu that shows the current profile/persona in the prompt.
 */
class CustomMenu : public cli::Menu
{
public:
    CustomMenu(const std::string& name, const std::string& desc = "(menu)")
        : cli::Menu(name, desc)
    {}

    /** @brief Inject the CoreInterface pointer for prompt rendering. */
    void setCore(CoreInterface* core) { m_core = core; };

    /** @brief Render the prompt with profile/persona info. */
    std::string Prompt() const
    {
        if( !m_core->isProfileOpen() )
            return cli::Menu::Prompt() + " (no profile)> ";

        QString persona = m_core->currentPersonaName();
        if( persona.isEmpty() )
            return cli::Menu::Prompt() + " (profile)> ";

        return cli::Menu::Prompt() + " (" + persona.toStdString() + ")> ";
    }

private:
    CoreInterface* m_core = nullptr;
};

/**
 * @brief Worker thread that runs the interactive CLI session.
 *
 * All data access goes through CoreInterface CRUD methods;
 * no direct database access from the UI layer.
 */
class UiWorker : public QThread
{
    Q_OBJECT

public:
    /** @brief Build the complete CLI menu tree. */
    void configure();
    void setCore(CoreInterface* core) { m_core = core; };

private:
    void run() override;

    // Menu builders
    void buildSettingsMenu();
    void buildInitCommand();
    void buildProfileMenu();
    void buildPersonaMenu();
    void buildContactMenu();
    void buildGroupMenu();
    void buildMessageMenu();
    void buildEventMenu();

    std::unique_ptr<CustomMenu> root_menu;
    CoreInterface* m_core = nullptr;
};

#endif // UIWORKER_H
