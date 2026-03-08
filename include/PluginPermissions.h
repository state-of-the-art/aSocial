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

#ifndef PLUGINPERMISSIONS_H
#define PLUGINPERMISSIONS_H

#include <QFlags>
#include <QStringList>

/**
 * @brief Granular capability flags that a plugin may request.
 *
 * Each flag represents a distinct category of Core functionality.
 * Plugins declare the set they need via PluginInterface::requiredPermissions();
 * the platform grants (or denies) each flag individually.  Unauthorised calls
 * through the CoreAccessProxy are logged and return safe default values.
 *
 * Flags are organised so that "Write" capabilities are always a superset of
 * the corresponding "Read" capability — granting SettingsWrite implicitly
 * grants SettingsRead during permission checking.
 */
enum class PluginPermission : quint32 {
    None = 0,

    /** @brief Read system settings (getSetting, listSettings). */
    SettingsRead = 1 << 0,

    /** @brief Write system settings (setSetting).  Implies SettingsRead. */
    SettingsWrite = 1 << 1,

    /** @brief Query container state (isContainerInitialized, containerPath). */
    ContainerRead = 1 << 2,

    /** @brief Create or modify VFS containers (createContainer).  Implies ContainerRead. */
    ContainerWrite = 1 << 3,

    /** @brief Read profile data and lifecycle state (getProfile, isProfileOpen, etc.). */
    ProfileRead = 1 << 4,

    /** @brief Modify profiles (openProfile, createProfile, storeProfile, closeProfile). */
    ProfileWrite = 1 << 5,

    /** @brief Delete the current profile (deleteCurrentProfile). */
    ProfileDelete = 1 << 6,

    /** @brief Export profiles (exportProfile). */
    ProfileExport = 1 << 7,

    /** @brief Import profiles (importProfile). */
    ProfileImport = 1 << 8,

    /** @brief Read entities: personas, contacts, groups, messages, events. */
    DataRead = 1 << 9,

    /** @brief Create, update and delete entities.  Implies DataRead. */
    DataWrite = 1 << 10,

    /** @brief Access raw plugin pointers (getDBKV, getDBKVProfile, getVFS). */
    PluginAccess = 1 << 11,

    /** @brief Control application lifecycle (exit). */
    AppLifecycle = 1 << 12,
};

Q_DECLARE_FLAGS(PluginPermissions, PluginPermission)
Q_DECLARE_OPERATORS_FOR_FLAGS(PluginPermissions)

/**
 * @brief Return a human-readable list of permission names from a bitmask.
 * @param perms  Permission flags to describe.
 * @return List of short labels such as "SettingsRead", "DataWrite", etc.
 */
inline QStringList permissionNames(PluginPermissions perms)
{
    QStringList names;
    if( perms.testFlag(PluginPermission::SettingsRead) )
        names << QStringLiteral("SettingsRead");
    if( perms.testFlag(PluginPermission::SettingsWrite) )
        names << QStringLiteral("SettingsWrite");
    if( perms.testFlag(PluginPermission::ContainerRead) )
        names << QStringLiteral("ContainerRead");
    if( perms.testFlag(PluginPermission::ContainerWrite) )
        names << QStringLiteral("ContainerWrite");
    if( perms.testFlag(PluginPermission::ProfileRead) )
        names << QStringLiteral("ProfileRead");
    if( perms.testFlag(PluginPermission::ProfileWrite) )
        names << QStringLiteral("ProfileWrite");
    if( perms.testFlag(PluginPermission::ProfileDelete) )
        names << QStringLiteral("ProfileDelete");
    if( perms.testFlag(PluginPermission::ProfileExport) )
        names << QStringLiteral("ProfileExport");
    if( perms.testFlag(PluginPermission::ProfileImport) )
        names << QStringLiteral("ProfileImport");
    if( perms.testFlag(PluginPermission::DataRead) )
        names << QStringLiteral("DataRead");
    if( perms.testFlag(PluginPermission::DataWrite) )
        names << QStringLiteral("DataWrite");
    if( perms.testFlag(PluginPermission::PluginAccess) )
        names << QStringLiteral("PluginAccess");
    if( perms.testFlag(PluginPermission::AppLifecycle) )
        names << QStringLiteral("AppLifecycle");
    if( names.isEmpty() )
        names << QStringLiteral("None");
    return names;
}

/**
 * @brief Expand implicit permissions (Write implies Read).
 *
 * Call this before storing the effective permission set so that runtime
 * checks only need to test single flags.
 *
 * @param perms  Raw permission flags as declared by the plugin.
 * @return Expanded flags with implied read permissions set.
 */
inline PluginPermissions expandImpliedPermissions(PluginPermissions perms)
{
    if( perms.testFlag(PluginPermission::SettingsWrite) )
        perms |= PluginPermission::SettingsRead;
    if( perms.testFlag(PluginPermission::ContainerWrite) )
        perms |= PluginPermission::ContainerRead;
    if( perms.testFlag(PluginPermission::DataWrite) )
        perms |= PluginPermission::DataRead;
    // ProfileWrite does NOT imply ProfileRead automatically — both must be declared
    // to keep profile-level access explicit for security auditing.
    return perms;
}

#endif // PLUGINPERMISSIONS_H
