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

#ifndef COREACCESSPROXY_H
#define COREACCESSPROXY_H

#include "CoreInterface.h"
#include "PluginPermissions.h"

#include <QObject>

/**
 * @brief Permission-gated proxy that wraps a real CoreInterface.
 *
 * Each plugin receives its own CoreAccessProxy instance configured with
 * the PluginPermissions that the platform has granted.  Every method
 * first checks the corresponding permission flag; denied calls are
 * logged with a warning and return safe default / error values.
 *
 * The proxy owns no resources — it holds a non-owning pointer to the
 * actual Core singleton whose lifetime is managed by the application.
 *
 * Temporary QByteArray buffers produced during export/import are
 * overwritten with random bytes before deallocation (secure wipe).
 */
class CoreAccessProxy : public QObject, public CoreInterface
{
    Q_OBJECT
    Q_INTERFACES(CoreInterface)

public:
    /**
     * @brief Construct a proxy for a specific plugin.
     * @param real         The real CoreInterface implementation (Core singleton).
     * @param granted      Permission flags granted to the plugin.
     * @param pluginName   Name of the plugin (for log messages).
     * @param parent       QObject parent for ownership.
     */
    explicit CoreAccessProxy(
        CoreInterface* real, PluginPermissions granted, const QString& pluginName, QObject* parent = nullptr);

    ~CoreAccessProxy() override;

    /** @brief Return the effective permission mask. */
    PluginPermissions grantedPermissions() const { return m_granted; }

    // ---- CoreInterface implementation (all gated) -------------------------

    // Plugin accessors
    DBKVPluginInterface* getDBKV() const override;
    DBKVPluginInterface* getDBKVProfile() const override;
    VFSPluginInterface* getVFS() const override;

    // Container lifecycle
    bool createContainer(const QString& path, quint64 maxSizeBytes = 0) override;
    bool isContainerInitialized() const override;
    QString containerPath() const override;

    // Profile lifecycle
    bool openProfile(const QString& password) override;
    bool createProfile(const QString& password, const QString& displayName) override;
    void closeProfile() override;
    bool isProfileOpen() const override;
    bool deleteCurrentProfile(std::function<void(int)> progress = nullptr) override;

    // Export / import
    QByteArray exportProfile(const QString& encryptionPassword = QString()) override;
    bool importProfile(const QByteArray& data, const QString& decryptionPassword, const QString& vfsPassword) override;

    // App Settings
    QVariant getSetting(const QString& key) const override;
    bool setSetting(const QString& key, const QVariant& value) override;
    QStringList listSettings() const override;

    // Profile data
    asocial::v1::Profile getProfile() const override;
    bool storeProfile(const asocial::v1::Profile& profile) override;
    QString getCurrentProfileId() const override;
    QString currentPersonaName() const override;

    // Profile param
    QList<asocial::v1::ProfileParameter> listParams() const override;
    asocial::v1::ProfileParameter createParam(const QString& paramKey) override;
    asocial::v1::ProfileParameter getParam(const QString& paramKey) const override;
    bool storeParam(const asocial::v1::ProfileParameter& param) override;
    bool deleteParam(const QString& paramKey) override;

    // Active persona
    bool setActivePersona(const QString& personaId) override;
    QString activePersonaId() const override;

    // Persona CRUD
    QList<asocial::v1::Persona> listPersonas() const override;
    asocial::v1::Persona createPersona(const QString& displayName) override;
    asocial::v1::Persona getPersona(const QString& personaId) const override;
    bool storePersona(const asocial::v1::Persona& persona) override;
    bool deletePersona(const QString& personaId) override;

    // Contact CRUD
    QList<asocial::v1::Contact> listContacts() const override;
    asocial::v1::Contact createContact(const QString& displayName) override;
    asocial::v1::Contact getContact(const QString& contactId) const override;
    bool storeContact(const asocial::v1::Contact& contact) override;
    bool deleteContact(const QString& contactId) override;
    QList<asocial::v1::Contact> searchContacts(const QString& query) const override;

    // Group CRUD
    QList<asocial::v1::Group> listGroups() const override;
    asocial::v1::Group createGroup(const QString& name) override;
    asocial::v1::Group getGroup(const QString& groupId) const override;
    bool storeGroup(const asocial::v1::Group& group) override;
    bool deleteGroup(const QString& groupId) override;
    bool addGroupMember(const QString& groupId, const QString& contactId) override;
    bool removeGroupMember(const QString& groupId, const QString& contactId) override;
    QList<asocial::v1::GroupMember> listGroupMembers(const QString& groupId) const override;

    // Message CRUD
    QList<asocial::v1::Message> listMessages(int limit = 50) const override;
    asocial::v1::Message createMessage(
        const QString& recipientId,
        const QString& body,
        const QString& recipientType = QStringLiteral("contact")) override;
    asocial::v1::Message getMessage(const QString& messageId) const override;
    bool storeMessage(const asocial::v1::Message& message) override;
    bool deleteMessage(const QString& messageId) override;

    // Event CRUD
    QList<asocial::v1::Event> listEvents() const override;
    asocial::v1::Event createEvent(const QString& title, const QString& date) override;
    asocial::v1::Event getEvent(const QString& eventId) const override;
    bool storeEvent(const asocial::v1::Event& event) override;
    bool deleteEvent(const QString& eventId) override;

    // App
    void exit() override;

signals:
    void profileChanged() override;

private:
    /**
     * @brief Check whether a single permission flag is granted.
     * @param perm  The capability to test.
     * @param method  Caller name for the log message.
     * @return true if permitted.
     */
    bool check(PluginPermission perm, const char* method) const;

    /** @brief Overwrite @p buf with random bytes, then clear it. */
    static void secureWipe(QByteArray& buf);

    CoreInterface* m_real;
    PluginPermissions m_granted;
    QString m_pluginName;
};

#endif // COREACCESSPROXY_H
