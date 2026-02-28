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

class CustomMenu : public cli::Menu
{
public:
    CustomMenu(const std::string& name, const std::string& desc = "(menu)")
        : cli::Menu(name, desc)
    {}

    void setCore(CoreInterface* core) { m_core = core; };

    std::string Prompt() const
    {
        QString currentAccountId = m_core->getCurrentProfileId();
        if( currentAccountId.isEmpty() ) {
            return cli::Menu::Prompt() + " (none)> ";
        } else {
            // Get account name
            QVariantMap info = m_core->getProfileInfo(currentAccountId);
            QString accountName = info.value("name", "unknown").toString();
            return cli::Menu::Prompt() + " (" + accountName.toStdString() + ")> ";
        }
    }

private:
    CoreInterface* m_core;
};

class UiWorker : public QThread
{
    Q_OBJECT

public:
    void configure();
    void setCore(CoreInterface* core) { m_core = core; };

private:
    void run() override;

    std::unique_ptr<CustomMenu> root_menu;
    CoreInterface* m_core;
};

#endif // UIWORKER_H
