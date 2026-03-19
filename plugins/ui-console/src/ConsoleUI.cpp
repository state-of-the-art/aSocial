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

#include "ConsoleUI.h"
#include "Log.h"

#include "console/CommandRouter.h"
#include "console/LineEditor.h"
#include "console/Prompt.h"
#include "console/SecureUtil.h"
#include "console/Spinner.h"
#include "console/Style.h"
#include "console/Terminal.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMetaObject>
#include <QStorageInfo>

#include <cstdlib>
#include <exception>

ConsoleUI::ConsoleUI() = default;
ConsoleUI::~ConsoleUI() = default;

namespace {
/**
 * @brief Format a protobuf Timestamp as an ISO-8601 date/time string.
 */
QString tsIso(const google::protobuf::Timestamp& ts)
{
    return QDateTime::fromMSecsSinceEpoch(ts.seconds() * 1000 + ts.nanos() / 1000000).toString(Qt::ISODate);
}

/** @brief Styled success message. */
void writeOk(Terminal& t, const QString& msg)
{
    t.writeLine(QStringLiteral("  %1%2%3 %4").arg(Style::success(), Style::checkmark, Style::reset(), msg));
}

/** @brief Styled error message. */
void writeErr(Terminal& t, const QString& msg)
{
    t.writeLine(QStringLiteral("  %1%2%3 %4").arg(Style::error(), Style::cross, Style::reset(), msg));
}

/** @brief Styled info message. */
void writeInfo(Terminal& t, const QString& msg)
{
    t.writeLine(QStringLiteral("  %1%2%3 %4").arg(Style::info(), Style::bullet, Style::reset(), msg));
}
} // namespace

// ===========================================================================
// Welcome banner
// ===========================================================================

void ConsoleUI::showBanner()
{
    const int w = 45;
    QString border = QString(w, QChar(0x2500)); // ─

    m_term->writeLine();
    m_term->writeLine(QStringLiteral("  %1%2%3%4").arg(Style::accent(), Style::boxTL, border, Style::boxTR));

    auto center = [&](const QString& text) {
        int pad = w - text.size();
        int left = pad / 2;
        int right = pad - left;
        return QString(left, QLatin1Char(' ')) + text + QString(right, QLatin1Char(' '));
    };

    m_term->writeLine(QStringLiteral("  %1%2%3%4aSocial  v%5%6%7%8")
                          .arg(
                              Style::accent(),
                              Style::boxV,
                              Style::reset(),
                              Style::bold(),
                              QStringLiteral(PROJECT_VERSION),
                              Style::reset(),
                              QString(w - 10 - QString(PROJECT_VERSION).size(), QLatin1Char(' ')),
                              QStringLiteral("%1%2%3").arg(Style::accent(), Style::boxV, Style::reset())));

    QString subtitle = QStringLiteral("Distributed Private Social Network");
    m_term->writeLine(
        QStringLiteral("  %1%2%3%4%5%1%2%3")
            .arg(Style::accent(), Style::boxV, Style::reset(), center(subtitle), QStringLiteral("%1").arg(Style::reset())));

    m_term->writeLine(QStringLiteral("  %1%2%3%4").arg(Style::accent(), Style::boxBL, border, Style::boxBR));

    m_term->writeLine(QStringLiteral("  %1Type '%2help%3%4' for available commands.%5")
                          .arg(Style::muted(), Style::reset(), Style::bold(), Style::muted(), Style::reset()));
    m_term->writeLine();
}

// ===========================================================================
// Prompt text  (e.g. "main/profile (Alice)> ")
// ===========================================================================

QString ConsoleUI::promptText() const
{
    QString path = m_router->currentPath();
    QString ctx;

    if( !m_core->isProfileOpen() )
        ctx = QStringLiteral(" %1(no profile)%2").arg(Style::muted(), Style::reset());
    else {
        QString persona = m_core->currentPersonaName();
        if( persona.isEmpty() )
            ctx = QStringLiteral(" %1(profile)%2").arg(Style::muted(), Style::reset());
        else
            ctx = QStringLiteral(" %1(%2)%3").arg(Style::cyan(), persona, Style::reset());
    }

    return QStringLiteral("%1%2%3%4%5 %6").arg(Style::bold(), Style::accent(), path, Style::reset(), ctx, Style::pointer);
}

// ===========================================================================
// Settings commands
// ===========================================================================

void ConsoleUI::buildSettingsCommands()
{
    m_router->addMenu(QString(), QStringLiteral("settings"), QStringLiteral("System-wide settings management"));

    m_router->addCommand(
        QStringLiteral("settings"),
        QStringLiteral("list"),
        QStringLiteral("List all settings"),
        {},
        [this](const QStringList&) {
            QStringList keys = m_core->listSettings();
            if( keys.isEmpty() ) {
                writeInfo(*m_term, QStringLiteral("No settings found."));
                return;
            }
            m_term->writeLine(
                QStringLiteral("  %1Settings (%2):%3").arg(Style::bold()).arg(keys.size()).arg(Style::reset()));
            for( const QString& key : keys ) {
                QVariant val = m_core->getSetting(key);
                m_term->writeLine(
                    QStringLiteral("    %1%2%3 = %4").arg(Style::accent(), key, Style::reset(), val.toString()));
            }
        });

    m_router->addCommand(
        QStringLiteral("settings"),
        QStringLiteral("get"),
        QStringLiteral("Get a setting value"),
        {QStringLiteral("key")},
        [this](const QStringList& args) {
            QString key = args.isEmpty() ? m_prompt->askText(QStringLiteral("Setting key")) : args[0];
            if( key.isEmpty() )
                return;
            QVariant val = m_core->getSetting(key);
            if( val.isNull() || !val.isValid() )
                writeInfo(*m_term, QStringLiteral("(not set)"));
            else
                m_term->writeLine(QStringLiteral("  %1").arg(val.toString()));
        });

    m_router->addCommand(
        QStringLiteral("settings"),
        QStringLiteral("set"),
        QStringLiteral("Set a setting value"),
        {QStringLiteral("key"), QStringLiteral("value")},
        [this](const QStringList& args) {
            QString key = args.size() > 0 ? args[0] : m_prompt->askText(QStringLiteral("Setting key"));
            if( key.isEmpty() )
                return;
            QString value = args.size() > 1 ? args[1] : m_prompt->askText(QStringLiteral("Value"));
            if( value.isNull() )
                return;
            m_core->setSetting(key, value);
            writeOk(*m_term, QStringLiteral("%1 = %2").arg(key, value));
        });
}

// ===========================================================================
// Init commands
// ===========================================================================

void ConsoleUI::buildInitCommands()
{
    m_router->addCommand(
        QString(),
        QStringLiteral("init"),
        QStringLiteral("Initialize new VFS storage"),
        {QStringLiteral("size_gb")},
        [this](const QStringList& args) {
            if( m_core->isContainerInitialized() ) {
                writeErr(*m_term, QStringLiteral("Container already exists at %1").arg(m_core->containerPath()));
                writeInfo(*m_term, QStringLiteral("Delete the file manually to replace it."));
                return;
            }

            QString path = m_core->getSetting(QStringLiteral("vfs.container.path")).toString();
            if( path.isEmpty() ) {
                writeErr(*m_term, QStringLiteral("Container path not configured."));
                return;
            }

            int sizeGb = 0;
            if( !args.isEmpty() ) {
                sizeGb = args[0].toInt();
            } else {
                QStorageInfo storage(
                    QFileInfo(path).absolutePath().isEmpty() ? QDir::currentPath() : QFileInfo(path).absolutePath());
                quint64 freeBytes = static_cast<quint64>(storage.bytesAvailable());
                quint64 proposed = qMin(freeBytes / 2, quint64(4ULL * 1024 * 1024 * 1024));
                int proposedGb = qMax(1, static_cast<int>(proposed / (1024ULL * 1024 * 1024)));

                writeInfo(*m_term, QStringLiteral("Available disk space: %1 MB").arg(freeBytes / (1024 * 1024)));
                sizeGb = m_prompt->askNumber(QStringLiteral("Container size in GB"), 1, 1024, proposedGb);
            }

            const quint64 sizeBytes = static_cast<quint64>(sizeGb) * 1024ULL * 1024ULL * 1024ULL;

            m_spinner->start(QStringLiteral("Creating VFS container (%1 GB)...").arg(sizeGb));
            bool ok = m_core->createContainer(path, sizeBytes);
            m_spinner->stop(
                ok,
                ok ? QStringLiteral("Container created at %1").arg(path) : QStringLiteral("Container creation failed"));

            if( ok )
                writeInfo(*m_term, QStringLiteral("Next step: create a profile with 'profile create'"));
        });

    m_router->addCommand(
        QString(),
        QStringLiteral("init_auto"),
        QStringLiteral("Show proposed container size"),
        {},
        [this](const QStringList&) {
            if( m_core->isContainerInitialized() ) {
                writeInfo(*m_term, QStringLiteral("Container already exists at %1").arg(m_core->containerPath()));
                return;
            }
            QString path = m_core->getSetting(QStringLiteral("vfs.container.path")).toString();
            if( path.isEmpty() )
                path = QStringLiteral("data.vfs");

            QStorageInfo storage(
                QFileInfo(path).absolutePath().isEmpty() ? QDir::currentPath() : QFileInfo(path).absolutePath());
            quint64 freeBytes = static_cast<quint64>(storage.bytesAvailable());
            quint64 proposed = qMin(freeBytes / 2, quint64(4ULL * 1024 * 1024 * 1024));
            int proposedMb = static_cast<int>(proposed / (1024 * 1024));

            writeInfo(*m_term, QStringLiteral("Available disk space: %1 MB").arg(freeBytes / (1024 * 1024)));
            writeInfo(*m_term, QStringLiteral("Proposed container size: %1 MB").arg(proposedMb));
            writeInfo(*m_term, QStringLiteral("To proceed, run: init %1").arg(proposedMb / 1024));
        });
}

// ===========================================================================
// Profile commands
// ===========================================================================

void ConsoleUI::buildProfileCommands()
{
    m_router->addMenu(QString(), QStringLiteral("profile"), QStringLiteral("Profile management (VFS-encrypted)"));

    m_router->addCommand(
        QStringLiteral("profile"),
        QStringLiteral("open"),
        QStringLiteral("Open a profile"),
        {QStringLiteral("password")},
        [this](const QStringList& args) {
            if( !m_core->isContainerInitialized() ) {
                writeErr(*m_term, QStringLiteral("No container found. Run 'init' first."));
                return;
            }

            QString password = args.isEmpty() ? m_prompt->askPassword(QStringLiteral("Enter password")) : args.value(0);
            SecureUtil::StringGuard pwGuard(password);
            if( password.isEmpty() )
                return;

            m_spinner->start(QStringLiteral("Opening profile (deriving key)..."));
            bool ok = m_core->openProfile(password);
            if( ok ) {
                auto prof = m_core->getProfile();
                m_spinner->stop(true, QStringLiteral("Profile opened: %1").arg(prof.displayName()));
                writeInfo(*m_term, QStringLiteral("UID: %1").arg(prof.uid()));
                writeInfo(*m_term, QStringLiteral("Created: %1").arg(tsIso(prof.createdAt())));
            } else {
                m_spinner
                    ->stop(false, QStringLiteral("Failed to open profile. Wrong password or no profile for this key."));
            }
        });

    m_router->addCommand(
        QStringLiteral("profile"),
        QStringLiteral("create"),
        QStringLiteral("Create a new profile"),
        {QStringLiteral("password"), QStringLiteral("display_name")},
        [this](const QStringList& args) {
            if( !m_core->isContainerInitialized() ) {
                writeErr(*m_term, QStringLiteral("No container found. Run 'init' first."));
                return;
            }

            QString password;
            if( args.size() >= 1 )
                password = args[0];
            else {
                password = m_prompt->askPassword(QStringLiteral("Enter password"));
                if( password.isEmpty() )
                    return;
                QString confirm = m_prompt->askPassword(QStringLiteral("Confirm password"));
                SecureUtil::StringGuard confirmGuard(confirm);
                if( password != confirm ) {
                    writeErr(*m_term, QStringLiteral("Passwords do not match."));
                    return;
                }
            }
            SecureUtil::StringGuard pwGuard(password);

            QString displayName = args.size() >= 2 ? args[1] : m_prompt->askText(QStringLiteral("Display name"));
            if( displayName.isEmpty() )
                return;

            m_spinner->start(QStringLiteral("Creating profile (deriving key)..."));
            bool ok = m_core->createProfile(password, displayName);
            if( ok ) {
                auto prof = m_core->getProfile();
                m_spinner->stop(true, QStringLiteral("Profile created: %1").arg(prof.displayName()));
                writeInfo(*m_term, QStringLiteral("UID: %1").arg(prof.uid()));
            } else {
                m_spinner->stop(false, QStringLiteral("Failed to create profile."));
            }
        });

    m_router->addCommand(
        QStringLiteral("profile"),
        QStringLiteral("current"),
        QStringLiteral("Show current profile info"),
        {},
        [this](const QStringList&) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile is currently open."));
                return;
            }
            auto prof = m_core->getProfile();
            m_term->writeLine(QStringLiteral("  %1Current profile:%2").arg(Style::bold(), Style::reset()));
            writeInfo(*m_term, QStringLiteral("UID:     %1").arg(prof.uid()));
            writeInfo(*m_term, QStringLiteral("Name:    %1").arg(prof.displayName()));
            writeInfo(*m_term, QStringLiteral("Bio:     %1").arg(prof.bio()));
            writeInfo(*m_term, QStringLiteral("Created: %1").arg(tsIso(prof.createdAt())));
            writeInfo(*m_term, QStringLiteral("Updated: %1").arg(tsIso(prof.updatedAt())));
            writeInfo(
                *m_term,
                QStringLiteral("Persona: %1 (%2)").arg(m_core->currentPersonaName(), m_core->activePersonaId()));
        });

    m_router->addCommand(
        QStringLiteral("profile"),
        QStringLiteral("update"),
        QStringLiteral("Update a profile field"),
        {QStringLiteral("field"), QStringLiteral("value")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile is open."));
                return;
            }

            QStringList fields = {QStringLiteral("display_name"), QStringLiteral("bio")};
            QString field;
            if( args.size() >= 1 ) {
                field = args[0];
            } else {
                int idx = m_prompt->askSelect(QStringLiteral("Select field to update"), fields);
                if( idx < 0 )
                    return;
                field = fields[idx];
            }

            QString value = args.size() >= 2 ? args[1] : m_prompt->askText(QStringLiteral("New value"));
            if( value.isNull() )
                return;

            auto prof = m_core->getProfile();
            if( field == QLatin1String("display_name") )
                prof.setDisplayName(value);
            else if( field == QLatin1String("bio") )
                prof.setBio(value);
            else {
                writeErr(*m_term, QStringLiteral("Unknown field: %1 (use: display_name, bio)").arg(field));
                return;
            }

            if( m_core->storeProfile(prof) )
                writeOk(*m_term, QStringLiteral("Profile updated: %1 = %2").arg(field, value));
            else
                writeErr(*m_term, QStringLiteral("Failed to update profile."));
        });

    m_router->addCommand(
        QStringLiteral("profile"),
        QStringLiteral("close"),
        QStringLiteral("Close the current profile"),
        {},
        [this](const QStringList&) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile is open."));
                return;
            }
            m_core->closeProfile();
            writeOk(*m_term, QStringLiteral("Profile closed."));
        });

    m_router->addCommand(
        QStringLiteral("profile"),
        QStringLiteral("delete"),
        QStringLiteral("Securely delete the current profile"),
        {},
        [this](const QStringList&) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile is open."));
                return;
            }
            if( !m_prompt->askConfirm(QStringLiteral("Securely delete this profile? This cannot be undone")) ) {
                writeInfo(*m_term, QStringLiteral("Cancelled."));
                return;
            }

            ProgressBar bar(*m_term, QStringLiteral("Deleting profile"), 30);
            bool ok = m_core->deleteCurrentProfile([&bar](int pct) { bar.update(pct); });
            bar.finish(ok);

            if( ok )
                writeOk(*m_term, QStringLiteral("Profile deleted and securely wiped."));
            else
                writeErr(*m_term, QStringLiteral("Failed to delete profile."));
        });

    m_router->addCommand(
        QStringLiteral("profile"),
        QStringLiteral("export"),
        QStringLiteral("Export profile to base64"),
        {},
        [this](const QStringList&) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile is open."));
                return;
            }
            QByteArray data = m_core->exportProfile();
            SecureUtil::Guard dataGuard(data);
            if( data.isEmpty() ) {
                writeErr(*m_term, QStringLiteral("Export failed."));
            } else {
                writeOk(*m_term, QStringLiteral("Exported profile data (base64):"));
                m_term->writeLine(QString::fromUtf8(data));
            }
        });

    m_router->addCommand(
        QStringLiteral("profile"),
        QStringLiteral("import"),
        QStringLiteral("Import profile from base64"),
        {QStringLiteral("base64_data"), QStringLiteral("vfs_password")},
        [this](const QStringList& args) {
            if( !m_core->isContainerInitialized() ) {
                writeErr(*m_term, QStringLiteral("No container found. Run 'init' first."));
                return;
            }
            QString data = args.size() >= 1 ? args[0]
                                            : m_prompt->askText(QStringLiteral("Base64-encoded profile data"));
            if( data.isEmpty() )
                return;

            QString vfsPw = args.size() >= 2 ? args[1]
                                             : m_prompt->askPassword(QStringLiteral("VFS password for new layer"));
            SecureUtil::StringGuard pwGuard(vfsPw);
            if( vfsPw.isEmpty() )
                return;

            m_spinner->start(QStringLiteral("Importing profile..."));
            bool ok = m_core->importProfile(data.toUtf8(), QString(), vfsPw);
            if( ok ) {
                auto prof = m_core->getProfile();
                m_spinner->stop(true, QStringLiteral("Profile imported: %1").arg(prof.displayName()));
            } else {
                m_spinner->stop(false, QStringLiteral("Import failed."));
            }
        });
}

// ===========================================================================
// Persona commands
// ===========================================================================

void ConsoleUI::buildPersonaCommands()
{
    m_router->addMenu(QString(), QStringLiteral("persona"), QStringLiteral("Manage personas within the current profile"));

    m_router->addCommand(
        QStringLiteral("persona"),
        QStringLiteral("list"),
        QStringLiteral("List all personas (* = active)"),
        {},
        [this](const QStringList&) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            auto personas = m_core->listPersonas();
            if( personas.isEmpty() ) {
                writeInfo(*m_term, QStringLiteral("No personas."));
                return;
            }
            m_term->writeLine(
                QStringLiteral("  %1Personas (%2):%3").arg(Style::bold()).arg(personas.size()).arg(Style::reset()));
            for( const auto& p : personas ) {
                bool active = (p.uid() == m_core->activePersonaId());
                QString marker = active
                                     ? QStringLiteral("%1%2%3 ").arg(Style::success(), Style::pointer, Style::reset())
                                     : QStringLiteral("  ");
                QString def = p.isDefault() ? QStringLiteral(" %1[default]%2").arg(Style::muted(), Style::reset())
                                            : QString();
                m_term->writeLine(
                    QStringLiteral("    %1%2%3  %4uid=%5%6")
                        .arg(marker, Style::bold(), p.displayName(), Style::muted(), p.uid(), Style::reset())
                    + def);
            }
        });

    m_router->addCommand(
        QStringLiteral("persona"),
        QStringLiteral("create"),
        QStringLiteral("Create a new persona"),
        {QStringLiteral("name")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            QString name = args.isEmpty() ? m_prompt->askText(QStringLiteral("Persona name")) : args[0];
            if( name.isEmpty() )
                return;

            auto persona = m_core->createPersona(name);
            if( persona.uid().isEmpty() || !m_core->storePersona(persona) ) {
                writeErr(*m_term, QStringLiteral("Failed to create persona."));
                return;
            }
            writeOk(*m_term, QStringLiteral("Persona created: %1 (uid=%2)").arg(name, persona.uid()));
        });

    m_router->addCommand(
        QStringLiteral("persona"),
        QStringLiteral("select"),
        QStringLiteral("Switch active persona"),
        {QStringLiteral("persona_id")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }

            QString personaId;
            if( !args.isEmpty() ) {
                personaId = args[0];
            } else {
                // Interactive selection
                auto personas = m_core->listPersonas();
                if( personas.isEmpty() ) {
                    writeInfo(*m_term, QStringLiteral("No personas available."));
                    return;
                }
                QStringList names;
                for( const auto& p : personas ) {
                    QString label = p.displayName();
                    if( p.isDefault() )
                        label += QStringLiteral(" [default]");
                    names.append(label);
                }
                int idx = m_prompt->askSelect(QStringLiteral("Select persona"), names);
                if( idx < 0 )
                    return;
                personaId = personas[idx].uid();
            }

            if( m_core->setActivePersona(personaId) )
                writeOk(*m_term, QStringLiteral("Active persona: %1").arg(m_core->currentPersonaName()));
            else
                writeErr(*m_term, QStringLiteral("Persona not found."));
        });

    m_router->addCommand(
        QStringLiteral("persona"),
        QStringLiteral("info"),
        QStringLiteral("Show persona details"),
        {QStringLiteral("persona_id")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            QString id = args.isEmpty() ? m_prompt->askText(QStringLiteral("Persona ID")) : args[0];
            if( id.isEmpty() )
                return;

            auto p = m_core->getPersona(id);
            if( p.uid().isEmpty() ) {
                writeErr(*m_term, QStringLiteral("Persona not found."));
                return;
            }
            m_term->writeLine(QStringLiteral("  %1Persona:%2").arg(Style::bold(), Style::reset()));
            writeInfo(*m_term, QStringLiteral("UID:     %1").arg(p.uid()));
            writeInfo(*m_term, QStringLiteral("Name:    %1").arg(p.displayName()));
            writeInfo(*m_term, QStringLiteral("Bio:     %1").arg(p.bio()));
            writeInfo(
                *m_term,
                QStringLiteral("Default: %1").arg(p.isDefault() ? QStringLiteral("yes") : QStringLiteral("no")));
            writeInfo(*m_term, QStringLiteral("Created: %1").arg(tsIso(p.createdAt())));
        });

    m_router->addCommand(
        QStringLiteral("persona"),
        QStringLiteral("update"),
        QStringLiteral("Update persona fields"),
        {QStringLiteral("persona_id"), QStringLiteral("field"), QStringLiteral("value")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            QString id = args.size() > 0 ? args[0] : m_prompt->askText(QStringLiteral("Persona ID"));
            if( id.isEmpty() )
                return;
            auto p = m_core->getPersona(id);
            if( p.uid().isEmpty() ) {
                writeErr(*m_term, QStringLiteral("Persona not found."));
                return;
            }

            QStringList fields = {QStringLiteral("name"), QStringLiteral("bio")};
            QString field;
            if( args.size() > 1 ) {
                field = args[1];
            } else {
                int idx = m_prompt->askSelect(QStringLiteral("Select field"), fields);
                if( idx < 0 )
                    return;
                field = fields[idx];
            }

            QString value = args.size() > 2 ? args[2] : m_prompt->askText(QStringLiteral("New value"));
            if( value.isNull() )
                return;

            if( field == QLatin1String("name") )
                p.setDisplayName(value);
            else if( field == QLatin1String("bio") )
                p.setBio(value);
            else {
                writeErr(*m_term, QStringLiteral("Unknown field: %1 (use: name, bio)").arg(field));
                return;
            }
            if( m_core->storePersona(p) )
                writeOk(*m_term, QStringLiteral("Persona updated."));
            else
                writeErr(*m_term, QStringLiteral("Failed to update persona."));
        });

    m_router->addCommand(
        QStringLiteral("persona"),
        QStringLiteral("delete"),
        QStringLiteral("Delete a persona"),
        {QStringLiteral("persona_id")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            QString id = args.isEmpty() ? m_prompt->askText(QStringLiteral("Persona ID")) : args[0];
            if( id.isEmpty() )
                return;
            if( id == m_core->activePersonaId() ) {
                writeErr(*m_term, QStringLiteral("Cannot delete the active persona. Switch first."));
                return;
            }
            if( !m_prompt->askConfirm(QStringLiteral("Delete this persona?")) )
                return;

            if( m_core->deletePersona(id) )
                writeOk(*m_term, QStringLiteral("Persona deleted."));
            else
                writeErr(*m_term, QStringLiteral("Failed to delete persona."));
        });
}

// ===========================================================================
// Contact commands
// ===========================================================================

void ConsoleUI::buildContactCommands()
{
    m_router->addMenu(QString(), QStringLiteral("contact"), QStringLiteral("Manage contacts for the active persona"));

    m_router->addCommand(
        QStringLiteral("contact"),
        QStringLiteral("list"),
        QStringLiteral("List all contacts"),
        {},
        [this](const QStringList&) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            auto contacts = m_core->listContacts();
            if( contacts.isEmpty() ) {
                writeInfo(*m_term, QStringLiteral("No contacts."));
                return;
            }
            m_term->writeLine(
                QStringLiteral("  %1Contacts (%2):%3").arg(Style::bold()).arg(contacts.size()).arg(Style::reset()));
            for( const auto& c : contacts ) {
                m_term->writeLine(QStringLiteral("    %1%2%3  trust=%4  %5uid=%6%7")
                                      .arg(Style::bold(), c.displayName(), Style::reset())
                                      .arg(static_cast<int>(c.trustLevel()))
                                      .arg(Style::muted(), c.uid(), Style::reset()));
            }
        });

    m_router->addCommand(
        QStringLiteral("contact"),
        QStringLiteral("add"),
        QStringLiteral("Add a new contact"),
        {QStringLiteral("name")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            QString name = args.isEmpty() ? m_prompt->askText(QStringLiteral("Contact name")) : args[0];
            if( name.isEmpty() )
                return;

            auto contact = m_core->createContact(name);
            if( contact.uid().isEmpty() || !m_core->storeContact(contact) ) {
                writeErr(*m_term, QStringLiteral("Failed to add contact."));
                return;
            }
            writeOk(*m_term, QStringLiteral("Contact added: %1 (uid=%2)").arg(name, contact.uid()));
        });

    m_router->addCommand(
        QStringLiteral("contact"),
        QStringLiteral("info"),
        QStringLiteral("Show contact details"),
        {QStringLiteral("contact_id")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            QString id = args.isEmpty() ? m_prompt->askText(QStringLiteral("Contact ID")) : args[0];
            if( id.isEmpty() )
                return;

            auto c = m_core->getContact(id);
            if( c.uid().isEmpty() ) {
                writeErr(*m_term, QStringLiteral("Contact not found."));
                return;
            }
            m_term->writeLine(QStringLiteral("  %1Contact:%2").arg(Style::bold(), Style::reset()));
            writeInfo(*m_term, QStringLiteral("UID:   %1").arg(c.uid()));
            writeInfo(*m_term, QStringLiteral("Name:  %1").arg(c.displayName()));
            writeInfo(*m_term, QStringLiteral("Trust: %1").arg(static_cast<int>(c.trustLevel())));
            if( c.hasNotes() )
                writeInfo(*m_term, QStringLiteral("Notes: %1").arg(c.notes()));
            writeInfo(
                *m_term,
                QStringLiteral("Key:   %1")
                    .arg(
                        c.publicKey().isEmpty() ? QStringLiteral("(none)")
                                                : QString::fromLatin1(c.publicKey().toHex())));
        });

    m_router->addCommand(
        QStringLiteral("contact"),
        QStringLiteral("update"),
        QStringLiteral("Update a contact field"),
        {QStringLiteral("contact_id"), QStringLiteral("field"), QStringLiteral("value")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            QString id = args.size() > 0 ? args[0] : m_prompt->askText(QStringLiteral("Contact ID"));
            if( id.isEmpty() )
                return;
            auto c = m_core->getContact(id);
            if( c.uid().isEmpty() ) {
                writeErr(*m_term, QStringLiteral("Contact not found."));
                return;
            }

            QStringList fields = {QStringLiteral("name"), QStringLiteral("notes"), QStringLiteral("trust")};
            QString field;
            if( args.size() > 1 ) {
                field = args[1];
            } else {
                int idx = m_prompt->askSelect(QStringLiteral("Select field"), fields);
                if( idx < 0 )
                    return;
                field = fields[idx];
            }

            QString value = args.size() > 2 ? args[2] : m_prompt->askText(QStringLiteral("New value"));
            if( value.isNull() )
                return;

            if( field == QLatin1String("name") )
                c.setDisplayName(value);
            else if( field == QLatin1String("notes") )
                c.setNotes(value);
            else if( field == QLatin1String("trust") )
                c.setTrustLevel(value.toInt());
            else {
                writeErr(*m_term, QStringLiteral("Unknown field: %1 (use: name, notes, trust)").arg(field));
                return;
            }

            if( m_core->storeContact(c) )
                writeOk(*m_term, QStringLiteral("Contact updated."));
            else
                writeErr(*m_term, QStringLiteral("Failed to update contact."));
        });

    m_router->addCommand(
        QStringLiteral("contact"),
        QStringLiteral("delete"),
        QStringLiteral("Delete a contact"),
        {QStringLiteral("contact_id")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            QString id = args.isEmpty() ? m_prompt->askText(QStringLiteral("Contact ID")) : args[0];
            if( id.isEmpty() )
                return;
            if( !m_prompt->askConfirm(QStringLiteral("Delete this contact?")) )
                return;

            if( m_core->deleteContact(id) )
                writeOk(*m_term, QStringLiteral("Contact deleted."));
            else
                writeErr(*m_term, QStringLiteral("Failed to delete contact."));
        });

    m_router->addCommand(
        QStringLiteral("contact"),
        QStringLiteral("search"),
        QStringLiteral("Search contacts"),
        {QStringLiteral("query")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            QString query = args.isEmpty() ? m_prompt->askText(QStringLiteral("Search query")) : args[0];
            if( query.isEmpty() )
                return;

            auto results = m_core->searchContacts(query);
            if( results.isEmpty() ) {
                writeInfo(*m_term, QStringLiteral("No matches."));
                return;
            }
            m_term->writeLine(
                QStringLiteral("  %1Found %2 contact(s):%3").arg(Style::bold()).arg(results.size()).arg(Style::reset()));
            for( const auto& c : results )
                m_term->writeLine(
                    QStringLiteral("    %1%2%3  %4uid=%5%6")
                        .arg(Style::bold(), c.displayName(), Style::reset(), Style::muted(), c.uid(), Style::reset()));
        });
}

// ===========================================================================
// Group commands
// ===========================================================================

void ConsoleUI::buildGroupCommands()
{
    m_router->addMenu(QString(), QStringLiteral("group"), QStringLiteral("Manage groups for the active persona"));

    m_router->addCommand(
        QStringLiteral("group"),
        QStringLiteral("list"),
        QStringLiteral("List all groups"),
        {},
        [this](const QStringList&) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            auto groups = m_core->listGroups();
            if( groups.isEmpty() ) {
                writeInfo(*m_term, QStringLiteral("No groups."));
                return;
            }
            m_term->writeLine(
                QStringLiteral("  %1Groups (%2):%3").arg(Style::bold()).arg(groups.size()).arg(Style::reset()));
            for( const auto& g : groups )
                m_term->writeLine(
                    QStringLiteral("    %1%2%3  %4uid=%5%6")
                        .arg(Style::bold(), g.name(), Style::reset(), Style::muted(), g.uid(), Style::reset()));
        });

    m_router->addCommand(
        QStringLiteral("group"),
        QStringLiteral("create"),
        QStringLiteral("Create a new group"),
        {QStringLiteral("name")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            QString name = args.isEmpty() ? m_prompt->askText(QStringLiteral("Group name")) : args[0];
            if( name.isEmpty() )
                return;

            auto group = m_core->createGroup(name);
            if( group.uid().isEmpty() || !m_core->storeGroup(group) ) {
                writeErr(*m_term, QStringLiteral("Failed to create group."));
                return;
            }
            writeOk(*m_term, QStringLiteral("Group created: %1 (uid=%2)").arg(name, group.uid()));
        });

    m_router->addCommand(
        QStringLiteral("group"),
        QStringLiteral("info"),
        QStringLiteral("Show group details"),
        {QStringLiteral("group_id")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            QString id = args.isEmpty() ? m_prompt->askText(QStringLiteral("Group ID")) : args[0];
            if( id.isEmpty() )
                return;

            auto g = m_core->getGroup(id);
            if( g.uid().isEmpty() ) {
                writeErr(*m_term, QStringLiteral("Group not found."));
                return;
            }
            auto members = m_core->listGroupMembers(g.uid());
            m_term->writeLine(QStringLiteral("  %1Group:%2").arg(Style::bold(), Style::reset()));
            writeInfo(*m_term, QStringLiteral("UID:     %1").arg(g.uid()));
            writeInfo(*m_term, QStringLiteral("Name:    %1").arg(g.name()));
            writeInfo(*m_term, QStringLiteral("Desc:    %1").arg(g.description()));
            writeInfo(*m_term, QStringLiteral("Members: %1").arg(members.size()));
            for( const auto& gm : members ) {
                auto contact = m_core->getContact(gm.contactUid());
                writeInfo(*m_term, QStringLiteral("  - %1").arg(contact.displayName()));
            }
        });

    m_router->addCommand(
        QStringLiteral("group"),
        QStringLiteral("update"),
        QStringLiteral("Update group fields"),
        {QStringLiteral("group_id"), QStringLiteral("field"), QStringLiteral("value")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            QString id = args.size() > 0 ? args[0] : m_prompt->askText(QStringLiteral("Group ID"));
            if( id.isEmpty() )
                return;
            auto g = m_core->getGroup(id);
            if( g.uid().isEmpty() ) {
                writeErr(*m_term, QStringLiteral("Group not found."));
                return;
            }

            QStringList fields = {QStringLiteral("name"), QStringLiteral("description")};
            QString field;
            if( args.size() > 1 ) {
                field = args[1];
            } else {
                int idx = m_prompt->askSelect(QStringLiteral("Select field"), fields);
                if( idx < 0 )
                    return;
                field = fields[idx];
            }

            QString value = args.size() > 2 ? args[2] : m_prompt->askText(QStringLiteral("New value"));
            if( value.isNull() )
                return;

            if( field == QLatin1String("name") )
                g.setName(value);
            else if( field == QLatin1String("description") )
                g.setDescription(value);
            else {
                writeErr(*m_term, QStringLiteral("Unknown field: %1").arg(field));
                return;
            }

            if( m_core->storeGroup(g) )
                writeOk(*m_term, QStringLiteral("Group updated."));
            else
                writeErr(*m_term, QStringLiteral("Failed."));
        });

    m_router->addCommand(
        QStringLiteral("group"),
        QStringLiteral("delete"),
        QStringLiteral("Delete a group"),
        {QStringLiteral("group_id")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            QString id = args.isEmpty() ? m_prompt->askText(QStringLiteral("Group ID")) : args[0];
            if( id.isEmpty() )
                return;
            if( !m_prompt->askConfirm(QStringLiteral("Delete this group?")) )
                return;

            if( m_core->deleteGroup(id) )
                writeOk(*m_term, QStringLiteral("Group deleted."));
            else
                writeErr(*m_term, QStringLiteral("Failed."));
        });

    m_router->addCommand(
        QStringLiteral("group"),
        QStringLiteral("add_member"),
        QStringLiteral("Add member to group"),
        {QStringLiteral("group_id"), QStringLiteral("contact_id")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            QString gid = args.size() > 0 ? args[0] : m_prompt->askText(QStringLiteral("Group ID"));
            if( gid.isEmpty() )
                return;
            QString cid = args.size() > 1 ? args[1] : m_prompt->askText(QStringLiteral("Contact ID"));
            if( cid.isEmpty() )
                return;

            if( m_core->addGroupMember(gid, cid) )
                writeOk(*m_term, QStringLiteral("Member added to group."));
            else
                writeErr(*m_term, QStringLiteral("Failed."));
        });

    m_router->addCommand(
        QStringLiteral("group"),
        QStringLiteral("remove_member"),
        QStringLiteral("Remove member from group"),
        {QStringLiteral("group_id"), QStringLiteral("contact_id")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            QString gid = args.size() > 0 ? args[0] : m_prompt->askText(QStringLiteral("Group ID"));
            if( gid.isEmpty() )
                return;
            QString cid = args.size() > 1 ? args[1] : m_prompt->askText(QStringLiteral("Contact ID"));
            if( cid.isEmpty() )
                return;

            if( m_core->removeGroupMember(gid, cid) )
                writeOk(*m_term, QStringLiteral("Member removed."));
            else
                writeErr(*m_term, QStringLiteral("Failed."));
        });
}

// ===========================================================================
// Message commands
// ===========================================================================

void ConsoleUI::buildMessageCommands()
{
    m_router->addMenu(QString(), QStringLiteral("message"), QStringLiteral("Send and read messages"));

    m_router->addCommand(
        QStringLiteral("message"),
        QStringLiteral("list"),
        QStringLiteral("List recent messages"),
        {},
        [this](const QStringList&) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            auto msgs = m_core->listMessages();
            if( msgs.isEmpty() ) {
                writeInfo(*m_term, QStringLiteral("No messages."));
                return;
            }
            m_term->writeLine(
                QStringLiteral("  %1Messages (%2):%3").arg(Style::bold()).arg(msgs.size()).arg(Style::reset()));
            for( const auto& m : msgs ) {
                QString sender = m.senderContactUid();
                QString bodyPreview = m.body().left(60);
                m_term->writeLine(QStringLiteral("    %1[%2]%3 %4 %5%6%7  %8uid=%9%10")
                                      .arg(
                                          Style::muted(),
                                          tsIso(m.createdAt()),
                                          Style::reset(),
                                          sender.isEmpty() ? QStringLiteral("self") : sender,
                                          Style::dim(),
                                          bodyPreview,
                                          m.body().size() > 60 ? QStringLiteral("...") : QString(),
                                          Style::muted(),
                                          m.uid(),
                                          Style::reset()));
            }
        });

    m_router->addCommand(
        QStringLiteral("message"),
        QStringLiteral("send"),
        QStringLiteral("Send a message"),
        {QStringLiteral("recipient_id"), QStringLiteral("body")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            QString recipientId = args.size() > 0 ? args[0] : m_prompt->askText(QStringLiteral("Recipient ID"));
            if( recipientId.isEmpty() )
                return;
            QString body = args.size() > 1 ? args[1] : m_prompt->askText(QStringLiteral("Message body"));
            if( body.isEmpty() )
                return;

            auto msg = m_core->createMessage(recipientId, body);
            if( msg.uid().isEmpty() || !m_core->storeMessage(msg) ) {
                writeErr(*m_term, QStringLiteral("Failed to send message."));
                return;
            }
            writeOk(*m_term, QStringLiteral("Message sent (uid=%1)").arg(msg.uid()));
        });

    m_router->addCommand(
        QStringLiteral("message"),
        QStringLiteral("read"),
        QStringLiteral("Read a message"),
        {QStringLiteral("message_id")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            QString id = args.isEmpty() ? m_prompt->askText(QStringLiteral("Message ID")) : args[0];
            if( id.isEmpty() )
                return;

            auto m = m_core->getMessage(id);
            if( m.uid().isEmpty() ) {
                writeErr(*m_term, QStringLiteral("Message not found."));
                return;
            }
            QString sender = m.senderContactUid();
            m_term->writeLine(QStringLiteral("  %1Message:%2").arg(Style::bold(), Style::reset()));
            writeInfo(*m_term, QStringLiteral("UID:    %1").arg(m.uid()));
            writeInfo(*m_term, QStringLiteral("Thread: %1").arg(m.threadUid()));
            writeInfo(*m_term, QStringLiteral("From:   %1").arg(sender.isEmpty() ? QStringLiteral("self") : sender));
            writeInfo(*m_term, QStringLiteral("To:     %1:%2").arg(m.recipientType(), m.recipientUid()));
            writeInfo(*m_term, QStringLiteral("Date:   %1").arg(tsIso(m.createdAt())));
            m_term->writeLine(QStringLiteral("  %1Body:%2").arg(Style::bold(), Style::reset()));
            m_term->writeLine(QStringLiteral("  %1").arg(m.body()));
        });

    m_router->addCommand(
        QStringLiteral("message"),
        QStringLiteral("delete"),
        QStringLiteral("Delete a message"),
        {QStringLiteral("message_id")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            QString id = args.isEmpty() ? m_prompt->askText(QStringLiteral("Message ID")) : args[0];
            if( id.isEmpty() )
                return;

            if( m_core->deleteMessage(id) )
                writeOk(*m_term, QStringLiteral("Message deleted."));
            else
                writeErr(*m_term, QStringLiteral("Failed."));
        });
}

// ===========================================================================
// Event commands
// ===========================================================================

void ConsoleUI::buildEventCommands()
{
    m_router->addMenu(QString(), QStringLiteral("event"), QStringLiteral("Manage timeline events"));

    m_router->addCommand(
        QStringLiteral("event"),
        QStringLiteral("list"),
        QStringLiteral("List all events"),
        {},
        [this](const QStringList&) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            auto events = m_core->listEvents();
            if( events.isEmpty() ) {
                writeInfo(*m_term, QStringLiteral("No events."));
                return;
            }
            m_term->writeLine(
                QStringLiteral("  %1Events (%2):%3").arg(Style::bold()).arg(events.size()).arg(Style::reset()));
            for( const auto& e : events ) {
                QString dateStr = e.hasStarted() ? tsIso(e.started()) : QString();
                m_term->writeLine(
                    QStringLiteral("    %1[%2]%3 %4  %5uid=%6%7")
                        .arg(Style::muted(), dateStr, Style::reset(), e.title(), Style::muted(), e.uid(), Style::reset()));
            }
        });

    m_router->addCommand(
        QStringLiteral("event"),
        QStringLiteral("create"),
        QStringLiteral("Create a new event"),
        {QStringLiteral("title"), QStringLiteral("date")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            QString title = args.size() > 0 ? args[0] : m_prompt->askText(QStringLiteral("Event title"));
            if( title.isEmpty() )
                return;
            QString date = args.size() > 1 ? args[1] : m_prompt->askText(QStringLiteral("Start date (YYYY-MM-DD)"));
            if( date.isEmpty() )
                return;

            auto event = m_core->createEvent(title, date);
            if( event.uid().isEmpty() || !m_core->storeEvent(event) ) {
                writeErr(*m_term, QStringLiteral("Failed to create event."));
                return;
            }
            writeOk(*m_term, QStringLiteral("Event created: %1 (uid=%2)").arg(title, event.uid()));
        });

    m_router->addCommand(
        QStringLiteral("event"),
        QStringLiteral("info"),
        QStringLiteral("Show event details"),
        {QStringLiteral("event_id")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            QString id = args.isEmpty() ? m_prompt->askText(QStringLiteral("Event ID")) : args[0];
            if( id.isEmpty() )
                return;

            auto e = m_core->getEvent(id);
            if( e.uid().isEmpty() ) {
                writeErr(*m_term, QStringLiteral("Event not found."));
                return;
            }
            m_term->writeLine(QStringLiteral("  %1Event:%2").arg(Style::bold(), Style::reset()));
            writeInfo(*m_term, QStringLiteral("UID:      %1").arg(e.uid()));
            writeInfo(*m_term, QStringLiteral("Title:    %1").arg(e.title()));
            writeInfo(*m_term, QStringLiteral("Date:     %1").arg(e.hasStarted() ? tsIso(e.started()) : QString()));
            writeInfo(*m_term, QStringLiteral("Location: %1").arg(e.location()));
            writeInfo(*m_term, QStringLiteral("Desc:     %1").arg(e.description()));
        });

    m_router->addCommand(
        QStringLiteral("event"),
        QStringLiteral("update"),
        QStringLiteral("Update event fields"),
        {QStringLiteral("event_id"), QStringLiteral("field"), QStringLiteral("value")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            QString id = args.size() > 0 ? args[0] : m_prompt->askText(QStringLiteral("Event ID"));
            if( id.isEmpty() )
                return;
            auto e = m_core->getEvent(id);
            if( e.uid().isEmpty() ) {
                writeErr(*m_term, QStringLiteral("Event not found."));
                return;
            }

            QStringList fields
                = {QStringLiteral("title"),
                   QStringLiteral("date"),
                   QStringLiteral("location"),
                   QStringLiteral("description")};
            QString field;
            if( args.size() > 1 ) {
                field = args[1];
            } else {
                int idx = m_prompt->askSelect(QStringLiteral("Select field"), fields);
                if( idx < 0 )
                    return;
                field = fields[idx];
            }

            QString value = args.size() > 2 ? args[2] : m_prompt->askText(QStringLiteral("New value"));
            if( value.isNull() )
                return;

            if( field == QLatin1String("title") )
                e.setTitle(value);
            else if( field == QLatin1String("description") )
                e.setDescription(value);
            else if( field == QLatin1String("location") )
                e.setLocation(value);
            else if( field == QLatin1String("date") ) {
                QDateTime dt = QDateTime::fromString(value, Qt::ISODate);
                if( !dt.isValid() )
                    dt = QDateTime::fromString(value, QStringLiteral("yyyy-MM-dd"));
                if( dt.isValid() ) {
                    google::protobuf::Timestamp ts;
                    ts.setSeconds(dt.toSecsSinceEpoch());
                    ts.setNanos(dt.time().msec() * 1000000);
                    e.setStarted(ts);
                } else {
                    writeErr(*m_term, QStringLiteral("Invalid date format. Use ISO-8601 or yyyy-MM-dd."));
                    return;
                }
            } else {
                writeErr(*m_term, QStringLiteral("Unknown field: %1").arg(field));
                return;
            }

            if( m_core->storeEvent(e) )
                writeOk(*m_term, QStringLiteral("Event updated."));
            else
                writeErr(*m_term, QStringLiteral("Failed to update event."));
        });

    m_router->addCommand(
        QStringLiteral("event"),
        QStringLiteral("delete"),
        QStringLiteral("Delete an event"),
        {QStringLiteral("event_id")},
        [this](const QStringList& args) {
            if( !m_core->isProfileOpen() ) {
                writeErr(*m_term, QStringLiteral("No profile open."));
                return;
            }
            QString id = args.isEmpty() ? m_prompt->askText(QStringLiteral("Event ID")) : args[0];
            if( id.isEmpty() )
                return;

            if( m_core->deleteEvent(id) )
                writeOk(*m_term, QStringLiteral("Event deleted."));
            else
                writeErr(*m_term, QStringLiteral("Failed."));
        });
}

// ===========================================================================
// Utility commands
// ===========================================================================

void ConsoleUI::buildUtilityCommands()
{
    m_router->addCommand(
        QString(), QStringLiteral("color"), QStringLiteral("Enable ANSI colours"), {}, [this](const QStringList&) {
            Style::setEnabled(true);
            writeOk(*m_term, QStringLiteral("Colours ON"));
        });

    m_router->addCommand(
        QString(), QStringLiteral("nocolor"), QStringLiteral("Disable ANSI colours"), {}, [this](const QStringList&) {
            Style::setEnabled(false);
            writeOk(*m_term, QStringLiteral("Colours OFF"));
        });
}

// ===========================================================================
// configure()
// ===========================================================================

void ConsoleUI::configure()
{
    LOG_D() << __func__;

    m_router = std::make_unique<CommandRouter>();

    buildSettingsCommands();
    buildInitCommands();
    buildProfileCommands();
    buildPersonaCommands();
    buildContactCommands();
    buildGroupCommands();
    buildMessageCommands();
    buildEventCommands();
    buildUtilityCommands();
}

// ===========================================================================
// Log queue (thread-safe; flushed so raw terminal is not broken)
// ===========================================================================

void ConsoleUI::enqueueLog(LogLevel level, const QString& message)
{
    QMutexLocker lock(&m_logMutex);
    m_logQueue.enqueue(qMakePair(level, message));
}

void ConsoleUI::flushLogQueue()
{
    if( !m_term )
        return;
    QQueue<QPair<LogLevel, QString>> pending;
    {
        QMutexLocker lock(&m_logMutex);
        pending.swap(m_logQueue);
    }
    for( const auto& p : std::as_const(pending) ) {
        LogLevel level = p.first;
        const QString& msg = p.second;
        const char* prefix = "";
        switch( level ) {
        case LogLevel::Debug:
            prefix = Style::muted();
            break;
        case LogLevel::Info:
            prefix = Style::info();
            break;
        case LogLevel::Warning:
            prefix = Style::warning();
            break;
        case LogLevel::Critical:
        case LogLevel::Fatal:
            prefix = Style::error();
            break;
        default:
            break;
        }
        m_term->writeLine(QStringLiteral("  %1%2%3").arg(QLatin1String(prefix), msg, QLatin1String(Style::reset())));
    }
}

// ===========================================================================
// run()  --  main read-eval-print loop
// ===========================================================================

void ConsoleUI::run()
{
    try {
        m_term = std::make_unique<Terminal>();
        m_editor = std::make_unique<LineEditor>(*m_term);
        m_prompt = std::make_unique<Prompt>(*m_term);
        m_spinner = std::make_unique<Spinner>(*m_term);

        m_term->enableRawMode();
        showBanner();

        for( ;; ) {
            flushLogQueue();

            QString prompt = promptText() + QStringLiteral(" ");
            QStringList completions = m_router->completions(QString());

            QString line = m_editor->readLine(prompt, completions);

            line = line.trimmed();
            if( line.isEmpty() )
                continue;

            m_editor->addToHistory(line);

            // Handle "help" directly so we can pass the terminal
            if( line == QLatin1String("help") ) {
                m_router->printHelp(*m_term);
                continue;
            }

            auto result = m_router->dispatch(line);

            switch( result ) {
            case CommandRouter::Result::Executed:
            case CommandRouter::Result::NavigatedMenu:
            case CommandRouter::Result::NavigatedUp:
            case CommandRouter::Result::Empty:
                continue;

            case CommandRouter::Result::Exit:
                m_term->writeLine(QStringLiteral("  Closing App..."));
                m_term->disableRawMode();
                m_core->exit();
                return;

            case CommandRouter::Result::NotFound:
                m_term->writeLine(QStringLiteral("  %1Unknown command: %2%3. Type 'help' for available commands.")
                                      .arg(Style::warning(), line, Style::reset()));
                continue;
            }
        }

        m_term->disableRawMode();
        m_core->exit();
    } catch( const std::bad_optional_access& ex ) {
        if( m_term ) {
            m_term->disableRawMode();
            writeErr(*m_term, QStringLiteral("Fatal error: bad optional access (%1)").arg(QString::fromLatin1(ex.what())));
        }
        try {
            if( m_core )
                m_core->closeProfile(); // Best-effort flush
        } catch( ... ) {
        }
        if( QCoreApplication::instance() ) {
            QMetaObject::invokeMethod(
                QCoreApplication::instance(), []() { QCoreApplication::exit(EXIT_FAILURE); }, Qt::QueuedConnection);
        } else {
            QCoreApplication::exit(EXIT_FAILURE);
        }
    } catch( const std::exception& ex ) {
        if( m_term ) {
            m_term->disableRawMode();
            writeErr(*m_term, QStringLiteral("Fatal error: %1").arg(QString::fromLatin1(ex.what())));
        }
        try {
            if( m_core )
                m_core->closeProfile(); // Best-effort flush
        } catch( ... ) {
        }
        if( QCoreApplication::instance() ) {
            QMetaObject::invokeMethod(
                QCoreApplication::instance(), []() { QCoreApplication::exit(EXIT_FAILURE); }, Qt::QueuedConnection);
        } else {
            QCoreApplication::exit(EXIT_FAILURE);
        }
    } catch( ... ) {
        if( m_term ) {
            m_term->disableRawMode();
            writeErr(*m_term, QStringLiteral("Fatal error: unknown exception"));
        }
        try {
            if( m_core )
                m_core->closeProfile(); // Best-effort flush
        } catch( ... ) {
        }
        if( QCoreApplication::instance() ) {
            QMetaObject::invokeMethod(
                QCoreApplication::instance(), []() { QCoreApplication::exit(EXIT_FAILURE); }, Qt::QueuedConnection);
        } else {
            QCoreApplication::exit(EXIT_FAILURE);
        }
    }
}
