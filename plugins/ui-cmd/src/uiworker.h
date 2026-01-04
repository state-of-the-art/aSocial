#ifndef UIWORKER_H
#define UIWORKER_H

#include <QThread>

#include "cli/cli.h"
#include "CoreInterface.h"

class CustomMenu : public cli::Menu
{
public:
    CustomMenu(const std::string& name, const std::string& desc = "(menu)")
        : cli::Menu(name, desc) {}

    void setCore(CoreInterface* core) { m_core = core; };

    std::string Prompt() const
    {
        QString currentAccountId = m_core->getCurrentProfileId();
        if (currentAccountId.isEmpty()) {
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
