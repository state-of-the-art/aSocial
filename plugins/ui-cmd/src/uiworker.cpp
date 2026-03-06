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

#include "uiworker.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QStorageInfo>

#include "qrcodegen.hpp"
#include <cli/cli.h>
#include <cli/clilocalsession.h>

#include "cli/loopscheduler.h"
namespace cli {
using MainScheduler = LoopScheduler;
} // namespace cli

Q_LOGGING_CATEGORY(Cw, PLUGIN_NAME "-worker")

// ---- Timestamp -> ISO string helper ----------------------------------------
namespace {
QString tsIso(const google::protobuf::Timestamp& ts)
{
    return QDateTime::fromMSecsSinceEpoch(ts.seconds() * 1000 + ts.nanos() / 1000000).toString(Qt::ISODate);
}
} // namespace

// ===================================================================
// configure()  -  build the complete CLI menu tree
// ===================================================================

void UiWorker::configure()
{
    qCDebug(Cw) << __func__;

    root_menu = std::make_unique<CustomMenu>("main", "Main menu of aSocial");
    root_menu->setCore(m_core);

    buildSettingsMenu();
    buildInitCommand();
    buildProfileMenu();
    buildPersonaMenu();
    buildContactMenu();
    buildGroupMenu();
    buildMessageMenu();
    buildEventMenu();

    // Utility commands
    root_menu->Insert(
        "qrcode",
        [](std::ostream& out, std::string text) {
            qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(text.data(), qrcodegen::QrCode::Ecc::MEDIUM);
            int border = 2;
            for( int y = -border; y < qr.getSize() + border; y += 2 ) {
                for( int x = -border; x < qr.getSize() + border; x++ ) {
                    bool top = qr.getModule(x, y);
                    bool bot = qr.getModule(x, y + 1);
                    if( top && bot )
                        out << " ";
                    else if( top )
                        out << "\xe2\x96\x84"; // ▄
                    else if( bot )
                        out << "\xe2\x96\x80"; // ▀
                    else
                        out << "\xe2\x96\x88"; // █
                }
                out << "\n";
            }
        },
        "Generate QR code: qrcode <text>");

    root_menu->Insert(
        "color",
        [](std::ostream& out) {
            out << "Colors ON\n";
            cli::SetColor();
        },
        "Enable colors in the cli");

    root_menu->Insert(
        "nocolor",
        [](std::ostream& out) {
            out << "Colors OFF\n";
            cli::SetNoColor();
        },
        "Disable colors in the cli");
}

// ===================================================================
// Settings submenu
// ===================================================================

void UiWorker::buildSettingsMenu()
{
    auto menu = std::make_unique<CustomMenu>("settings", "System-wide settings management");
    menu->setCore(m_core);

    menu->Insert(
        "list",
        [this](std::ostream& out) {
            QStringList keys = m_core->listSettings();
            if( keys.isEmpty() ) {
                out << "No settings found.\n";
                return;
            }
            out << "Settings (" << keys.size() << "):\n";
            for( const QString& key : keys ) {
                QVariant val = m_core->getSetting(key);
                out << "  " << key.toStdString() << " = " << val.toString().toStdString() << "\n";
            }
        },
        "List all settings with current values");

    menu->Insert(
        "get",
        [this](std::ostream& out, std::string key) {
            QVariant val = m_core->getSetting(QString::fromStdString(key));
            if( val.isNull() || !val.isValid() )
                out << "(not set)\n";
            else
                out << val.toString().toStdString() << "\n";
        },
        "Get a setting: get <key>");

    menu->Insert(
        "set",
        [this](std::ostream& out, std::string key, std::string value) {
            m_core->setSetting(QString::fromStdString(key), QString::fromStdString(value));
            out << "OK: " << key << " = " << value << "\n";
        },
        "Set a setting: set <key> <value>");

    root_menu->Insert(std::unique_ptr<cli::Menu>(std::move(menu)));
}

// ===================================================================
// init command - create data.vfs container
// ===================================================================

void UiWorker::buildInitCommand()
{
    root_menu->Insert(
        "init",
        [this](std::ostream& out, int sizeMb) {
            if( m_core->isContainerInitialized() ) {
                out << "WARNING: Container already exists at " << m_core->containerPath().toStdString() << "\n"
                    << "To replace it, delete the file manually first.\n";
                return;
            }

            QString path = m_core->getSetting("vfs.container.path").toString();
            if( path.isEmpty() )
                path = QStringLiteral("data.vfs");

            const quint64 sizeBytes = static_cast<quint64>(sizeMb) * 1024ULL * 1024ULL;

            out << "Creating VFS container: " << path.toStdString() << " (" << sizeMb << " MB)...\n";
            out << "  Stage 1/3: Allocating space and filling with random data...\n";
            out.flush();

            if( !m_core->createContainer(path, sizeBytes) ) {
                out << "ERROR: Container creation failed.\n";
                return;
            }

            out << "  Stage 2/3: Container created successfully.\n"
                << "  Stage 3/3: Ready for profile creation.\n\n"
                << "Container initialized at " << path.toStdString() << "\n"
                << "Next step: create a profile with  profile create <password> <display_name>\n";
        },
        "Initialize new VFS storage: init <size_mb>");

    root_menu->Insert(
        "init_auto",
        [this](std::ostream& out) {
            if( m_core->isContainerInitialized() ) {
                out << "Container already exists at " << m_core->containerPath().toStdString() << "\n";
                return;
            }

            QString path = m_core->getSetting("vfs.container.path").toString();
            if( path.isEmpty() )
                path = QStringLiteral("data.vfs");

            QStorageInfo storage(
                QFileInfo(path).absolutePath().isEmpty() ? QDir::currentPath() : QFileInfo(path).absolutePath());
            quint64 freeBytes = static_cast<quint64>(storage.bytesAvailable());
            quint64 proposed = qMin(freeBytes / 2, quint64(4ULL * 1024 * 1024 * 1024));
            int proposedMb = static_cast<int>(proposed / (1024 * 1024));

            out << "Available disk space: " << (freeBytes / (1024 * 1024)) << " MB\n"
                << "Proposed container size: " << proposedMb << " MB\n"
                << "To proceed, run:  init " << proposedMb << "\n";
        },
        "Show proposed container size based on available disk space");
}

// ===================================================================
// Profile submenu
// ===================================================================

void UiWorker::buildProfileMenu()
{
    auto menu = std::make_unique<CustomMenu>("profile", "Profile management (VFS-encrypted)");
    menu->setCore(m_core);

    menu->Insert(
        "open",
        [this](std::ostream& out, std::string password) {
            if( !m_core->isContainerInitialized() ) {
                out << "No container found. Run 'init <size_mb>' first.\n";
                return;
            }
            out << "Opening profile (deriving key, this may take a moment)...\n";
            out.flush();
            if( m_core->openProfile(QString::fromStdString(password)) ) {
                auto prof = m_core->getProfile();
                out << "Profile opened: " << prof.displayName().toStdString() << "\n"
                    << "  ID: " << prof.uid().toStdString() << "\n"
                    << "  Created: " << tsIso(prof.createdAt()).toStdString() << "\n";
            } else {
                out << "Failed to open profile. Wrong password or no profile for this key.\n";
            }
        },
        "Open a profile: open <password>");

    menu->Insert(
        "create",
        [this](std::ostream& out, std::string password, std::string displayName) {
            if( !m_core->isContainerInitialized() ) {
                out << "No container found. Run 'init <size_mb>' first.\n";
                return;
            }
            out << "Creating profile (deriving key, this may take a moment)...\n";
            out.flush();
            if( m_core->createProfile(QString::fromStdString(password), QString::fromStdString(displayName)) ) {
                auto prof = m_core->getProfile();
                out << "Profile created successfully!\n"
                    << "  Name: " << prof.displayName().toStdString() << "\n"
                    << "  ID: " << prof.uid().toStdString() << "\n";
            } else {
                out << "Failed to create profile.\n";
            }
        },
        "Create a new profile: create <password> <display_name>");

    menu->Insert(
        "current",
        [this](std::ostream& out) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile is currently open.\n";
                return;
            }
            auto prof = m_core->getProfile();
            out << "Current profile:\n"
                << "  ID:      " << prof.uid().toStdString() << "\n"
                << "  Name:    " << prof.displayName().toStdString() << "\n"
                << "  Bio:     " << prof.bio().toStdString() << "\n"
                << "  Created: " << tsIso(prof.createdAt()).toStdString() << "\n"
                << "  Updated: " << tsIso(prof.updatedAt()).toStdString() << "\n"
                << "  Persona: " << m_core->currentPersonaName().toStdString() << " ("
                << m_core->activePersonaId().toStdString() << ")\n";
        },
        "Show current profile and persona info");

    menu->Insert(
        "update",
        [this](std::ostream& out, std::string field, std::string value) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile is open.\n";
                return;
            }
            auto prof = m_core->getProfile();
            QString f = QString::fromStdString(field);
            QString v = QString::fromStdString(value);
            if( f == QLatin1String("display_name") )
                prof.setDisplayName(v);
            else if( f == QLatin1String("bio") )
                prof.setBio(v);
            else {
                out << "Unknown field: " << field << " (use: display_name, bio)\n";
                return;
            }
            if( m_core->storeProfile(prof) )
                out << "Profile updated: " << field << " = " << value << "\n";
            else
                out << "Failed to update profile.\n";
        },
        "Update profile field: update <field> <value>  (fields: display_name, bio)");

    menu->Insert(
        "close",
        [this](std::ostream& out) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile is open.\n";
                return;
            }
            m_core->closeProfile();
            out << "Profile closed.\n";
        },
        "Close the current profile");

    menu->Insert(
        "delete",
        [this](std::ostream& out) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile is open. Open it first with 'profile open <password>'.\n";
                return;
            }
            out << "Securely deleting profile...\n";
            out.flush();
            bool ok = m_core->deleteCurrentProfile([&out](int pct) {
                out << "  Progress: " << pct << "%\n";
                out.flush();
            });
            if( ok )
                out << "Profile deleted and securely wiped.\n";
            else
                out << "Failed to delete profile.\n";
        },
        "Securely delete the current profile from VFS");

    menu->Insert(
        "export",
        [this](std::ostream& out) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile is open.\n";
                return;
            }
            QByteArray data = m_core->exportProfile();
            if( data.isEmpty() ) {
                out << "Export failed.\n";
            } else {
                out << "Exported profile data (base64, plaintext JSON):\n" << data.toStdString() << "\n";
            }
        },
        "Export current profile to base64 JSON");

    menu->Insert(
        "import",
        [this](std::ostream& out, std::string base64data, std::string vfsPassword) {
            if( !m_core->isContainerInitialized() ) {
                out << "No container found. Run 'init <size_mb>' first.\n";
                return;
            }
            out << "Importing profile...\n";
            out.flush();
            if( m_core->importProfile(
                    QByteArray::fromStdString(base64data), QString(), QString::fromStdString(vfsPassword)) ) {
                auto prof = m_core->getProfile();
                out << "Profile imported: " << prof.displayName().toStdString() << "\n";
            } else {
                out << "Import failed.\n";
            }
        },
        "Import profile: import <base64_data> <vfs_password>");

    root_menu->Insert(std::unique_ptr<cli::Menu>(std::move(menu)));
}

// ===================================================================
// Persona submenu
// ===================================================================

void UiWorker::buildPersonaMenu()
{
    auto menu = std::make_unique<CustomMenu>("persona", "Manage personas within the current profile");
    menu->setCore(m_core);

    menu->Insert(
        "list",
        [this](std::ostream& out) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            auto personas = m_core->listPersonas();
            if( personas.isEmpty() ) {
                out << "No personas.\n";
                return;
            }
            out << "Personas (" << personas.size() << "):\n";
            for( const auto& p : personas ) {
                out << "  " << (p.uid() == m_core->activePersonaId() ? "* " : "  ") << p.displayName().toStdString()
                    << (p.isDefault() ? " [default]" : "") << "  id=" << p.uid().toStdString() << "\n";
            }
        },
        "List all personas (* = active)");

    menu->Insert(
        "create",
        [this](std::ostream& out, std::string name) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            auto persona = m_core->createPersona(QString::fromStdString(name));
            if( persona.uid().isEmpty() || !m_core->storePersona(persona) ) {
                out << "Failed to create persona.\n";
                return;
            }
            out << "Persona created: " << name << " (id=" << persona.uid().toStdString() << ")\n";
        },
        "Create persona: create <name>");

    menu->Insert(
        "select",
        [this](std::ostream& out, std::string personaId) {
            if( m_core->setActivePersona(QString::fromStdString(personaId)) )
                out << "Active persona: " << m_core->currentPersonaName().toStdString() << "\n";
            else
                out << "Persona not found.\n";
        },
        "Switch active persona: select <persona_id>");

    menu->Insert(
        "info",
        [this](std::ostream& out, std::string personaId) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            auto p = m_core->getPersona(QString::fromStdString(personaId));
            if( p.uid().isEmpty() ) {
                out << "Persona not found.\n";
                return;
            }
            out << "Persona:\n"
                << "  ID:      " << p.uid().toStdString() << "\n"
                << "  Name:    " << p.displayName().toStdString() << "\n"
                << "  Bio:     " << p.bio().toStdString() << "\n"
                << "  Default: " << (p.isDefault() ? "yes" : "no") << "\n"
                << "  Created: " << tsIso(p.createdAt()).toStdString() << "\n";
        },
        "Show persona details: info <persona_id>");

    menu->Insert(
        "update",
        [this](std::ostream& out, std::string personaId, std::string field, std::string value) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            auto p = m_core->getPersona(QString::fromStdString(personaId));
            if( p.uid().isEmpty() ) {
                out << "Persona not found.\n";
                return;
            }
            QString f = QString::fromStdString(field);
            QString v = QString::fromStdString(value);
            if( f == QLatin1String("name") )
                p.setDisplayName(v);
            else if( f == QLatin1String("bio") )
                p.setBio(v);
            else {
                out << "Unknown field: " << field << " (use: name, bio)\n";
                return;
            }
            if( m_core->storePersona(p) )
                out << "Persona updated.\n";
            else
                out << "Failed to update persona.\n";
        },
        "Update persona: update <persona_id> <field> <value>  (fields: name, bio)");

    menu->Insert(
        "delete",
        [this](std::ostream& out, std::string personaId) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            if( personaId == m_core->activePersonaId().toStdString() ) {
                out << "Cannot delete the active persona. Switch first.\n";
                return;
            }
            if( m_core->deletePersona(QString::fromStdString(personaId)) )
                out << "Persona deleted.\n";
            else
                out << "Failed to delete persona.\n";
        },
        "Delete persona: delete <persona_id>");

    root_menu->Insert(std::unique_ptr<cli::Menu>(std::move(menu)));
}

// ===================================================================
// Contact submenu
// ===================================================================

void UiWorker::buildContactMenu()
{
    auto menu = std::make_unique<CustomMenu>("contact", "Manage contacts for the active persona");
    menu->setCore(m_core);

    menu->Insert(
        "list",
        [this](std::ostream& out) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            auto contacts = m_core->listContacts();
            if( contacts.isEmpty() ) {
                out << "No contacts.\n";
                return;
            }
            out << "Contacts (" << contacts.size() << "):\n";
            for( const auto& c : contacts ) {
                out << "  " << c.displayName().toStdString() << "  trust=" << c.trustLevel()
                    << "  id=" << c.uid().toStdString() << "\n";
            }
        },
        "List all contacts of the active persona");

    menu->Insert(
        "add",
        [this](std::ostream& out, std::string name) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            auto contact = m_core->createContact(QString::fromStdString(name));
            if( contact.uid().isEmpty() || !m_core->storeContact(contact) ) {
                out << "Failed to add contact.\n";
                return;
            }
            out << "Contact added: " << name << " (id=" << contact.uid().toStdString() << ")\n";
        },
        "Add contact: add <name>");

    menu->Insert(
        "info",
        [this](std::ostream& out, std::string contactId) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            auto c = m_core->getContact(QString::fromStdString(contactId));
            if( c.uid().isEmpty() ) {
                out << "Contact not found.\n";
                return;
            }
            out << "Contact:\n"
                << "  ID:    " << c.uid().toStdString() << "\n"
                << "  Name:  " << c.displayName().toStdString() << "\n"
                << "  Trust: " << c.trustLevel() << "\n"
                << "  Notes: " << c.notes().toStdString() << "\n"
                << "  Key:   "
                << (c.publicKey().isEmpty() ? "(none)" : QString::fromLatin1(c.publicKey().toHex()).toStdString())
                << "\n";
        },
        "Show contact details: info <contact_id>");

    menu->Insert(
        "update",
        [this](std::ostream& out, std::string contactId, std::string field, std::string value) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            auto c = m_core->getContact(QString::fromStdString(contactId));
            if( c.uid().isEmpty() ) {
                out << "Contact not found.\n";
                return;
            }
            QString f = QString::fromStdString(field);
            QString v = QString::fromStdString(value);
            if( f == QLatin1String("name") )
                c.setDisplayName(v);
            else if( f == QLatin1String("notes") )
                c.setNotes(v);
            else if( f == QLatin1String("trust") )
                c.setTrustLevel(v.toInt());
            else {
                out << "Unknown field: " << field << " (use: name, notes, trust)\n";
                return;
            }
            if( m_core->storeContact(c) )
                out << "Contact updated.\n";
            else
                out << "Failed to update contact.\n";
        },
        "Update contact: update <contact_id> <field> <value>");

    menu->Insert(
        "delete",
        [this](std::ostream& out, std::string contactId) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            if( m_core->deleteContact(QString::fromStdString(contactId)) )
                out << "Contact deleted.\n";
            else
                out << "Failed to delete contact.\n";
        },
        "Delete contact: delete <contact_id>");

    menu->Insert(
        "search",
        [this](std::ostream& out, std::string query) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            auto results = m_core->searchContacts(QString::fromStdString(query));
            if( results.isEmpty() ) {
                out << "No matches.\n";
                return;
            }
            out << "Found " << results.size() << " contact(s):\n";
            for( const auto& c : results ) {
                out << "  " << c.displayName().toStdString() << "  id=" << c.uid().toStdString() << "\n";
            }
        },
        "Search contacts: search <query>");

    root_menu->Insert(std::unique_ptr<cli::Menu>(std::move(menu)));
}

// ===================================================================
// Group submenu
// ===================================================================

void UiWorker::buildGroupMenu()
{
    auto menu = std::make_unique<CustomMenu>("group", "Manage groups for the active persona");
    menu->setCore(m_core);

    menu->Insert(
        "list",
        [this](std::ostream& out) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            auto groups = m_core->listGroups();
            if( groups.isEmpty() ) {
                out << "No groups.\n";
                return;
            }
            out << "Groups (" << groups.size() << "):\n";
            for( const auto& g : groups ) {
                out << "  " << g.name().toStdString() << "  id=" << g.uid().toStdString() << "\n";
            }
        },
        "List all groups");

    menu->Insert(
        "create",
        [this](std::ostream& out, std::string name) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            auto group = m_core->createGroup(QString::fromStdString(name));
            if( group.uid().isEmpty() || !m_core->storeGroup(group) ) {
                out << "Failed to create group.\n";
                return;
            }
            out << "Group created: " << name << " (id=" << group.uid().toStdString() << ")\n";
        },
        "Create group: create <name>");

    menu->Insert(
        "info",
        [this](std::ostream& out, std::string groupId) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            auto g = m_core->getGroup(QString::fromStdString(groupId));
            if( g.uid().isEmpty() ) {
                out << "Group not found.\n";
                return;
            }
            auto members = m_core->listGroupMembers(g.uid());
            out << "Group:\n"
                << "  ID:      " << g.uid().toStdString() << "\n"
                << "  Name:    " << g.name().toStdString() << "\n"
                << "  Desc:    " << g.description().toStdString() << "\n"
                << "  Members: " << members.size() << "\n";
            for( const auto& gm : members ) {
                auto contact = m_core->getContact(gm.contactUid());
                out << "    - " << contact.displayName().toStdString() << "\n";
            }
        },
        "Show group details: info <group_id>");

    menu->Insert(
        "update",
        [this](std::ostream& out, std::string groupId, std::string field, std::string value) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            auto g = m_core->getGroup(QString::fromStdString(groupId));
            if( g.uid().isEmpty() ) {
                out << "Group not found.\n";
                return;
            }
            QString f = QString::fromStdString(field);
            QString v = QString::fromStdString(value);
            if( f == QLatin1String("name") )
                g.setName(v);
            else if( f == QLatin1String("description") )
                g.setDescription(v);
            else {
                out << "Unknown field: " << field << " (use: name, description)\n";
                return;
            }
            if( m_core->storeGroup(g) )
                out << "Group updated.\n";
            else
                out << "Failed to update group.\n";
        },
        "Update group: update <group_id> <field> <value>  (fields: name, description)");

    menu->Insert(
        "delete",
        [this](std::ostream& out, std::string groupId) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            if( m_core->deleteGroup(QString::fromStdString(groupId)) )
                out << "Group deleted.\n";
            else
                out << "Failed.\n";
        },
        "Delete group: delete <group_id>");

    menu->Insert(
        "add_member",
        [this](std::ostream& out, std::string groupId, std::string contactId) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            if( m_core->addGroupMember(QString::fromStdString(groupId), QString::fromStdString(contactId)) )
                out << "Member added to group.\n";
            else
                out << "Failed.\n";
        },
        "Add member to group: add_member <group_id> <contact_id>");

    menu->Insert(
        "remove_member",
        [this](std::ostream& out, std::string groupId, std::string contactId) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            if( m_core->removeGroupMember(QString::fromStdString(groupId), QString::fromStdString(contactId)) )
                out << "Member removed.\n";
            else
                out << "Failed.\n";
        },
        "Remove member: remove_member <group_id> <contact_id>");

    root_menu->Insert(std::unique_ptr<cli::Menu>(std::move(menu)));
}

// ===================================================================
// Message submenu
// ===================================================================

void UiWorker::buildMessageMenu()
{
    auto menu = std::make_unique<CustomMenu>("message", "Send and read messages");
    menu->setCore(m_core);

    menu->Insert(
        "list",
        [this](std::ostream& out) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            auto msgs = m_core->listMessages();
            if( msgs.isEmpty() ) {
                out << "No messages.\n";
                return;
            }
            out << "Messages (" << msgs.size() << "):\n";
            for( const auto& m : msgs ) {
                QString sender = m.senderContactUid();
                QString bodyPreview = m.body().left(60);
                out << "  [" << tsIso(m.createdAt()).toStdString() << "] "
                    << (sender.isEmpty() ? "self" : sender.toStdString()) << " -> " << m.recipientType().toStdString()
                    << ":" << m.recipientUid().toStdString() << "  " << bodyPreview.toStdString()
                    << (m.body().size() > 60 ? "..." : "") << "  id=" << m.uid().toStdString() << "\n";
            }
        },
        "List recent messages");

    menu->Insert(
        "send",
        [this](std::ostream& out, std::string recipientId, std::string body) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            auto msg = m_core->createMessage(QString::fromStdString(recipientId), QString::fromStdString(body));
            if( msg.uid().isEmpty() || !m_core->storeMessage(msg) ) {
                out << "Failed to send message.\n";
                return;
            }
            out << "Message sent (id=" << msg.uid().toStdString() << ")\n";
        },
        "Send message: send <recipient_id> <body>");

    menu->Insert(
        "read",
        [this](std::ostream& out, std::string messageId) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            auto m = m_core->getMessage(QString::fromStdString(messageId));
            if( m.uid().isEmpty() ) {
                out << "Message not found.\n";
                return;
            }
            QString sender = m.senderContactUid();
            out << "Message:\n"
                << "  ID:        " << m.uid().toStdString() << "\n"
                << "  Thread:    " << m.threadUid().toStdString() << "\n"
                << "  From:      " << (sender.isEmpty() ? "self" : sender.toStdString()) << "\n"
                << "  To:        " << m.recipientType().toStdString() << ":" << m.recipientUid().toStdString() << "\n"
                << "  Date:      " << tsIso(m.createdAt()).toStdString() << "\n"
                << "  Body:\n"
                << m.body().toStdString() << "\n";
        },
        "Read a message: read <message_id>");

    menu->Insert(
        "delete",
        [this](std::ostream& out, std::string messageId) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            if( m_core->deleteMessage(QString::fromStdString(messageId)) )
                out << "Message deleted.\n";
            else
                out << "Failed.\n";
        },
        "Delete message: delete <message_id>");

    root_menu->Insert(std::unique_ptr<cli::Menu>(std::move(menu)));
}

// ===================================================================
// Event submenu
// ===================================================================

void UiWorker::buildEventMenu()
{
    auto menu = std::make_unique<CustomMenu>("event", "Manage timeline events");
    menu->setCore(m_core);

    menu->Insert(
        "list",
        [this](std::ostream& out) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            auto events = m_core->listEvents();
            if( events.isEmpty() ) {
                out << "No events.\n";
                return;
            }
            out << "Events (" << events.size() << "):\n";
            for( const auto& e : events ) {
                QString dateStr = e.hasStarted() ? tsIso(e.started()) : QString();
                out << "  [" << dateStr.toStdString() << "] " << e.title().toStdString()
                    << "  id=" << e.uid().toStdString() << "\n";
            }
        },
        "List all events");

    menu->Insert(
        "create",
        [this](std::ostream& out, std::string title, std::string date) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            auto event = m_core->createEvent(QString::fromStdString(title), QString::fromStdString(date));
            if( event.uid().isEmpty() || !m_core->storeEvent(event) ) {
                out << "Failed to create event.\n";
                return;
            }
            out << "Event created: " << title << " (id=" << event.uid().toStdString() << ")\n";
        },
        "Create event: create <title> <date>");

    menu->Insert(
        "info",
        [this](std::ostream& out, std::string eventId) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            auto e = m_core->getEvent(QString::fromStdString(eventId));
            if( e.uid().isEmpty() ) {
                out << "Event not found.\n";
                return;
            }
            out << "Event:\n"
                << "  ID:       " << e.uid().toStdString() << "\n"
                << "  Title:    " << e.title().toStdString() << "\n"
                << "  Date:     " << (e.hasStarted() ? tsIso(e.started()).toStdString() : std::string()) << "\n"
                << "  Location: " << e.location().toStdString() << "\n"
                << "  Desc:     " << e.description().toStdString() << "\n";
        },
        "Show event details: info <event_id>");

    menu->Insert(
        "update",
        [this](std::ostream& out, std::string eventId, std::string field, std::string value) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            auto e = m_core->getEvent(QString::fromStdString(eventId));
            if( e.uid().isEmpty() ) {
                out << "Event not found.\n";
                return;
            }
            QString f = QString::fromStdString(field);
            QString v = QString::fromStdString(value);
            if( f == QLatin1String("title") )
                e.setTitle(v);
            else if( f == QLatin1String("description") )
                e.setDescription(v);
            else if( f == QLatin1String("location") )
                e.setLocation(v);
            else if( f == QLatin1String("date") ) {
                QDateTime dt = QDateTime::fromString(v, Qt::ISODate);
                if( !dt.isValid() )
                    dt = QDateTime::fromString(v, QStringLiteral("yyyy-MM-dd"));
                if( dt.isValid() ) {
                    google::protobuf::Timestamp ts;
                    ts.setSeconds(dt.toSecsSinceEpoch());
                    ts.setNanos(dt.time().msec() * 1000000);
                    e.setStarted(ts);
                }
            } else {
                out << "Unknown field: " << field << " (use: title, date, location, description)\n";
                return;
            }
            if( m_core->storeEvent(e) )
                out << "Event updated.\n";
            else
                out << "Failed to update event.\n";
        },
        "Update event: update <event_id> <field> <value>  (fields: title, date, location, description)");

    menu->Insert(
        "delete",
        [this](std::ostream& out, std::string eventId) {
            if( !m_core->isProfileOpen() ) {
                out << "No profile open.\n";
                return;
            }
            if( m_core->deleteEvent(QString::fromStdString(eventId)) )
                out << "Event deleted.\n";
            else
                out << "Failed.\n";
        },
        "Delete event: delete <event_id>");

    root_menu->Insert(std::unique_ptr<cli::Menu>(std::move(menu)));
}

// ===================================================================
// run()  -  start the CLI session
// ===================================================================

void UiWorker::run()
{
    qCDebug(Cw) << __func__;

    cli::Cli cli(std::move(root_menu));

    cli.ExitAction([](auto& out) { out << "Stopping the UI CLI worker.\n"; });
    cli.StdExceptionHandler([](std::ostream& out, const std::string& cmd, const std::exception& e) {
        out << "Exception caught in CLI handler: " << e.what() << " while handling command: " << cmd << ".\n";
    });
    cli.WrongCommandHandler([](std::ostream& out, const std::string& cmd) {
        out << "Unknown command or incorrect parameters: " << cmd << ".\n";
    });

    cli::MainScheduler scheduler;
    cli::CliLocalTerminalSession localSession(cli, scheduler, std::cout, 200);
    localSession.ExitAction([&scheduler](auto& out) {
        out << "Closing App...\n";
        scheduler.Stop();
    });

    scheduler.Run();

    m_core->exit();
}
