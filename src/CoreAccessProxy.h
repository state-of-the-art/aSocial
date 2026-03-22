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
 * Each plugin receives its own CoreAccessProxy configured with
 * PluginPermissions flags (which map to the generated AccessCategory
 * system). Every method checks the corresponding permission; denied
 * calls are logged and return safe defaults.
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

    PluginPermissions grantedPermissions() const { return m_granted; }

    // ---- CoreInterface implementation (all gated) -------------------------

    DBKVPluginInterface* getDBKV() const override;
    DBKVPluginInterface* getDBKVProfile() const override;
    VFSPluginInterface* getVFS() const override;

    bool createContainer(const QString& path, quint64 maxSizeBytes = 0) override;
    bool isContainerInitialized() const override;
    QString containerPath() const override;

    bool openProfile(const QString& password) override;
    bool createProfile(const QString& password, const QString& displayName) override;
    void closeProfile() override;
    bool isProfileOpen() const override;
    bool deleteCurrentProfile(std::function<void(int)> progress = nullptr) override;

    QByteArray exportProfile(const QString& encryptionPassword = QString()) override;
    bool importProfile(const QByteArray& data, const QString& decryptionPassword, const QString& vfsPassword) override;

    QVariant getSetting(const QString& key) const override;
    bool setSetting(const QString& key, const QVariant& value) override;
    QStringList listSettings() const override;

    asocial::v1::Profile getProfile() const override;
    bool storeProfile(const asocial::v1::Profile& profile) override;
    QString getCurrentProfileId() const override;
    QString currentPersonaName() const override;

    QList<asocial::v1::ProfileParameter> listParams() const override;
    asocial::v1::ProfileParameter createParam(const QString& paramKey) override;
    asocial::v1::ProfileParameter getParam(const QString& paramKey) const override;
    bool storeParam(const asocial::v1::ProfileParameter& param) override;
    bool deleteParam(const QString& paramKey) override;

    bool setActivePersona(const QString& personaId) override;
    QString activePersonaId() const override;

    QList<asocial::v1::Persona> listPersonas() const override;
    asocial::v1::Persona createPersona(const QString& displayName) override;
    asocial::v1::Persona getPersona(const QString& personaId) const override;
    bool storePersona(const asocial::v1::Persona& persona) override;
    bool deletePersona(const QString& personaId) override;

    QList<asocial::v1::Contact> listContacts() const override;
    asocial::v1::Contact createContact(const QString& displayName) override;
    asocial::v1::Contact getContact(const QString& contactId) const override;
    bool storeContact(const asocial::v1::Contact& contact) override;
    bool deleteContact(const QString& contactId) override;
    QList<asocial::v1::Contact> searchContacts(const QString& query) const override;

    QList<asocial::v1::Group> listGroups() const override;
    asocial::v1::Group createGroup(const QString& name) override;
    asocial::v1::Group getGroup(const QString& groupId) const override;
    bool storeGroup(const asocial::v1::Group& group) override;
    bool deleteGroup(const QString& groupId) override;
    bool addGroupMember(const QString& groupId, const QString& contactId) override;
    bool removeGroupMember(const QString& groupId, const QString& contactId) override;
    QList<asocial::v1::GroupMember> listGroupMembers(const QString& groupId) const override;

    QList<asocial::v1::Message> listMessages(int limit = 50) const override;
    asocial::v1::Message createMessage(
        const QString& recipientId,
        const QString& body,
        const QString& recipientType = QStringLiteral("contact")) override;
    asocial::v1::Message getMessage(const QString& messageId) const override;
    bool storeMessage(const asocial::v1::Message& message) override;
    bool deleteMessage(const QString& messageId) override;

    QList<asocial::v1::Event> listEvents() const override;
    asocial::v1::Event createEvent(const QString& title, const QString& date) override;
    asocial::v1::Event getEvent(const QString& eventId) const override;
    bool storeEvent(const asocial::v1::Event& event) override;
    bool deleteEvent(const QString& eventId) override;

    void exit() override;

signals:
    void profileChanged() override;

private:
    bool check(PluginPermission perm, const char* method) const;
    static void secureWipe(QByteArray& buf);

    CoreInterface* m_real;
    PluginPermissions m_granted;
    QString m_pluginName;
};

#endif // COREACCESSPROXY_H
