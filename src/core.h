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

#ifndef CORE_H
#define CORE_H

#include "CoreInterface.h"
#include "plugin/DBKVPluginInterface.h"
#include "plugin/VFS/VFSContainerPluginInterface.h"
#include "plugin/VFSPluginInterface.h"

#include <QObject>

class QCoreApplication;

/**
 * @brief Application-wide singleton managing the plugin chain and profile lifecycle.
 *
 * Storage pipeline:
 *   VFS-cake container (data.vfs)  ->  DBKV-rocksdb (protobuf KV store)
 *
 * The VFS container holds one encrypted "file" per encryption layer.
 * Each layer corresponds to one profile, identified by its passphrase.
 * All entity data is serialised with protobuf and stored under a
 * hierarchical key scheme for efficient prefix scans.
 *
 * CRUD operations use protobuf-generated types directly.  Factory
 * methods (create*) return populated but **unsaved** objects; store*
 * methods persist them to the DBKV database and flush immediately.
 */
class Core : public QObject, public CoreInterface
{
    Q_OBJECT
    Q_INTERFACES(CoreInterface)

public:
    inline static Core* I()
    {
        if( s_pInstance == nullptr )
            s_pInstance = new Core();
        return s_pInstance;
    }
    inline static void destroyI() { delete s_pInstance; }

    // ---- Plugin wiring (called from main) --------------------------------
    void setDBKVPlugin(const QString& pluginName);
    void setDBKVProfilePlugin(const QString& pluginName);
    void setVFSPlugin(const QString& pluginName);
    void setApp(QCoreApplication* app);

    // ---- CoreInterface implementation ------------------------------------

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

    // Profile data
    asocial::v1::Profile getProfile() const override;
    bool storeProfile(const asocial::v1::Profile& profile) override;
    QString getCurrentProfileId() const override;
    QString currentPersonaName() const override;

    // Export / import
    QByteArray exportProfile(const QString& encryptionPassword = QString()) override;
    bool importProfile(const QByteArray& data, const QString& decryptionPassword, const QString& vfsPassword) override;

    // Settings
    QVariant getSetting(const QString& key) const override;
    // On purpose can't write settings through core to prevent plugins from manipulating
    bool setSetting(const QString& key, const QVariant& value);
    QStringList listSettings() const override;

    // Persona
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
        const QString& recipientId, const QString& body, const QString& recipientType) override;
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
    explicit Core(QObject* parent = nullptr);
    ~Core() override;

    /**
     * @brief Derive the DBKV virtual-file name from the profile DBKV plugin.
     *
     * Convention: "<plugin-name>.dat", e.g. "dbkv-rocksdb.dat".
     */
    QString dbFileName() const;

    /** @brief Generate a fresh UUID without braces. */
    static QString newId();

    /** @brief Overwrite @p buf with random bytes, then clear it. */
    static void secureWipe(QByteArray& buf);

    /** @brief Flush profile DB and emit profileChanged(). */
    void flushProfile();

    static Core* s_pInstance;

    QCoreApplication* m_app = nullptr;

    // Plugin pointers (non-owning, managed by Plugins singleton)
    DBKVPluginInterface* m_dbkv = nullptr;        ///< Background/relay KV (dbkv-json)
    DBKVPluginInterface* m_dbkvProfile = nullptr; ///< Encrypted profile KV (dbkv-rocksdb)
    VFSPluginInterface* m_vfs = nullptr;

    // Runtime state
    VFSContainerPluginInterface* m_container = nullptr; ///< Currently open VFS container
    QIODevice* m_dbDevice = nullptr;                    ///< Virtual file backing the DBKV profile

    QString m_containerPath;    ///< Filesystem path to data.vfs
    QString m_currentProfileId; ///< UUID of the open profile
    QString m_activePersonaId;  ///< UUID of the impersonated persona
};

#endif // CORE_H
