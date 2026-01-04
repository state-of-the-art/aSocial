#include "uiworker.h"

#include <QLoggingCategory>
#include <QDir>
#include <QFileInfo>

#include <cli/cli.h>
#include <cli/clilocalsession.h>
#include "qrcodegen.hpp"

#include "cli/loopscheduler.h"
namespace cli
{
    using MainScheduler = LoopScheduler;
} // namespace cli

Q_LOGGING_CATEGORY(Cw, PLUGIN_NAME "-worker")

void UiWorker::configure()
{
    qCDebug(Cw) << __func__;

    root_menu = std::make_unique<CustomMenu>("main", "Main menu of aSocial");
    root_menu->setCore(m_core);

    // Profile commands
    std::unique_ptr<CustomMenu> profileMenu = std::make_unique<CustomMenu>("profile", "Control profiles on the device");
    profileMenu->setCore(m_core);

    profileMenu->Insert(
        "list",
        [this](std::ostream& out) {
            QStringList profiles = m_core->listProfiles();
            if (profiles.isEmpty()) {
                out << "No profiles found\n";
            } else {
                out << "Profiles:\n";
                for (const QString &profileId : profiles) {
                    QVariantMap info = m_core->getProfileInfo(profileId);
                    out << "  ID: " << profileId.toStdString()
                        << " | Name: " << info["name"].toString().toStdString()
                        << " | Created: " << info["created"].toDateTime().toString().toStdString() << "\n";
                }
            }
        },
        "List all profiles" );

    profileMenu->Insert(
        "create",
        [this](std::ostream& out, std::string name) {
            QString profileId = m_core->createProfile(QString::fromStdString(name));
            if (profileId.isEmpty()) {
                out << "Failed to create profile\n";
            } else {
                out << "Profile '" << name << "' created with ID: " << profileId.toStdString() << "\n";
            }
        },
        "Create a new profile: create <name>" );

    profileMenu->Insert(
        "export",
        [this](std::ostream& out, std::string profileId) {
            QString data = m_core->exportProfile(QString::fromStdString(profileId));
            if (data.isEmpty()) {
                out << "Failed to export profile\n";
            } else {
                out << "Profile data: " << data.toStdString() << "\n";
            }
        },
        "Export profile: export <profile_id>" );

    profileMenu->Insert(
        "import",
        [this](std::ostream& out, std::string data) {
            QString profileId = m_core->importProfile(QString::fromStdString(data));
            if (profileId.isEmpty()) {
                out << "Failed to import profile\n";
            } else {
                out << "Profile imported with ID: " << profileId.toStdString() << "\n";
            }
        },
        "Import profile: import <base64_data>" );

    profileMenu->Insert(
        "delete",
        [this](std::ostream& out, std::string profileId) {
            bool success = m_core->deleteProfile(QString::fromStdString(profileId));
            if (success) {
                out << "Profile deleted: " << profileId << "\n";
            } else {
                out << "Failed to delete profile\n";
            }
        },
        "Delete profile: delete <profile_id>" );

    profileMenu->Insert(
        "set",
        [this](std::ostream& out, std::string profileId) {
            m_core->setCurrentProfileId(QString::fromStdString(profileId));
            QVariantMap info = m_core->getProfileInfo(QString::fromStdString(profileId));
            if (info.isEmpty()) {
                out << "Profile not found\n";
            } else {
                out << "Current profile set to: " << info["name"].toString().toStdString() << "\n";
            }
        },
        "Set current profile: set <profile_id>" );

    profileMenu->Insert(
        "current",
        [this](std::ostream& out) {
            QString currentId = m_core->getCurrentProfileId();
            if (currentId.isEmpty()) {
                out << "No current profile set\n";
            } else {
                QVariantMap info = m_core->getProfileInfo(currentId);
                if (info.isEmpty()) {
                    out << "Current profile not found\n";
                } else {
                    out << "Current profile:\n";
                    out << "  ID: " << currentId.toStdString() << "\n";
                    out << "  Name: " << info["name"].toString().toStdString() << "\n";
                    out << "  Created: " << info["created"].toDateTime().toString().toStdString() << "\n";
                }
            }
        },
        "Show current profile information" );

    root_menu->Insert( std::unique_ptr<cli::Menu>(std::move(profileMenu)) );

    root_menu->Insert(
            "hello",
            [](std::ostream& out){ out << "Hello, world\n"; },
            "Print hello world" );
    root_menu->Insert(
            "qrcodetest",
            [](std::ostream& out, std::string text) {
                out << "QR code:\n";
                qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(text.data(), qrcodegen::QrCode::Ecc::MEDIUM);

                int border = 2;
                for (int y = -border; y < qr.getSize() + border; y+=2) {
                    for (int x = -border; x < qr.getSize() + border; x++) {
                        if( qr.getModule(x, y) && qr.getModule(x, y+1) ) {
                            out << " ";
                        } else if( qr.getModule(x, y) ) {
                            out << "▄";
                        } else if( qr.getModule(x, y+1) ) {
                            out << "▀";
                        } else {
                            out << "█";
                        }
                    }
                    out << std::endl;
                }
            },
            "Print the example qr code" ),
    root_menu->Insert(
            "hello_everysession",
            [](std::ostream&){ cli::Cli::cout() << "Hello, everybody" << std::endl; },
            "Print hello everybody on all open sessions" );
    root_menu->Insert(
            "answer",
            [](std::ostream& out, int x){ out << "The answer is: " << x << "\n"; },
            "Print the answer to Life, the Universe and Everything " );
    root_menu->Insert(
            "color",
            [](std::ostream& out){ out << "Colors ON\n"; cli::SetColor(); },
            "Enable colors in the cli" );
    root_menu->Insert(
            "nocolor",
            [](std::ostream& out){ out << "Colors OFF\n"; cli::SetNoColor(); },
            "Disable colors in the cli" );
}

void UiWorker::run()
{
    qCDebug(Cw) << __func__;

    cli::Cli cli(std::move(root_menu));
    // global exit action
    cli.ExitAction([](auto& out){ out << "Stopping the UI CLI worker.\n"; });
    cli.StdExceptionHandler(
        [](std::ostream& out, const std::string& cmd, const std::exception& e)
        {
            out << "Exception caught in CLI handler: "
                << e.what()
                << " while handling command: "
                << cmd
                << ".\n";
        }
    );
    // custom handler for unknown commands
    cli.WrongCommandHandler(
        [](std::ostream& out, const std::string& cmd)
        {
            out << "Unknown command or incorrect parameters: "
                << cmd
                << ".\n";
        }
    );

    cli::MainScheduler scheduler;
    cli::CliLocalTerminalSession localSession(cli, scheduler, std::cout, 200);
    localSession.ExitAction(
        [&scheduler](auto& out) // session exit action
        {
            out << "Closing App...\n";
            scheduler.Stop();
        }
        );

    scheduler.Run();
}
