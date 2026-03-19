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

#ifndef GUIBACKEND_H
#define GUIBACKEND_H

#include "CoreInterface.h"

#include <QDateTime>
#include <QObject>
#include <QRandomGenerator>
#include <QVariantList>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

/**
 * @brief QML-facing bridge that exposes CoreInterface operations as
 *        invokable methods and notifiable properties.
 *
 * All data flows through QVariantMap / QVariantList so that QML can
 * consume the results without requiring protobuf type registration in
 * the QML engine.  Temporary QByteArrays holding sensitive data are
 * securely wiped before deallocation.
 */
class GuiBackend : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool containerInitialized READ containerInitialized NOTIFY stateChanged)
    Q_PROPERTY(bool profileOpen READ profileOpen NOTIFY stateChanged)
    Q_PROPERTY(QString profileName READ profileName NOTIFY stateChanged)
    Q_PROPERTY(QString profileId READ profileId NOTIFY stateChanged)
    Q_PROPERTY(QString activePersonaId READ activePersonaId NOTIFY stateChanged)
    Q_PROPERTY(QString activePersonaName READ activePersonaName NOTIFY stateChanged)
    Q_PROPERTY(QVariantMap currentProfile READ currentProfile NOTIFY stateChanged)

public:
    explicit GuiBackend(QObject* parent = nullptr);
    ~GuiBackend() override;

    /**
     * @brief Inject the CoreInterface proxy (called once from Plugin::startUI).
     * @param core  Permission-gated proxy, non-owning.
     */
    void setCore(CoreInterface* core);

    // ---- Property getters ------------------------------------------------

    /** @brief True when a VFS container file exists on disk. */
    bool containerInitialized() const;

    /** @brief True when an encrypted profile is open and active. */
    bool profileOpen() const;

    /** @brief Display name of the current profile (empty if none). */
    QString profileName() const;

    /** @brief UID of the current profile (empty if none). */
    QString profileId() const;

    /** @brief UID of the active persona within the profile. */
    QString activePersonaId() const;

    /** @brief Display name of the active persona. */
    QString activePersonaName() const;

    /** @brief Full profile as a QVariantMap for QML binding. */
    QVariantMap currentProfile() const;

    // ---- Container / Profile lifecycle (Q_INVOKABLE for QML) -------------

    /** @brief Create a new VFS container at the configured path. */
    Q_INVOKABLE bool initContainer(int sizeMb);

    /** @brief Open a profile with the given password. */
    Q_INVOKABLE bool openProfile(const QString& password);

    /** @brief Create a new profile inside the container. */
    Q_INVOKABLE bool createProfile(const QString& password, const QString& displayName);

    /** @brief Close the currently open profile. */
    Q_INVOKABLE void closeProfile();

    /** @brief Securely delete the current profile. */
    Q_INVOKABLE bool deleteProfile();

    /** @brief Export current profile to base64. */
    Q_INVOKABLE QString exportProfile();

    /** @brief Import a profile from base64 data. */
    Q_INVOKABLE bool importProfile(const QString& data, const QString& vfsPassword);

    // ---- Settings --------------------------------------------------------

    /** @brief Read a system-wide setting. */
    Q_INVOKABLE QVariant getSetting(const QString& key) const;

    /** @brief Write a system-wide setting. */
    Q_INVOKABLE bool setSetting(const QString& key, const QVariant& value);

    /** @brief List all setting keys. */
    Q_INVOKABLE QStringList listSettings() const;

    // ---- Profile params --------------------------------------------------

    /** @brief List all profile parameters as [{key, value}, ...]. */
    Q_INVOKABLE QVariantList listParams() const;

    /** @brief Get a single profile parameter value. */
    Q_INVOKABLE QString getParam(const QString& key) const;

    /** @brief Set a profile parameter. */
    Q_INVOKABLE bool setParam(const QString& key, const QString& value);

    /** @brief Delete a profile parameter. */
    Q_INVOKABLE bool deleteParam(const QString& key);

    // ---- Persona CRUD ----------------------------------------------------

    /** @brief List all personas as QVariantList of QVariantMaps. */
    Q_INVOKABLE QVariantList listPersonas() const;

    /** @brief Create and store a new persona. Returns its UID. */
    Q_INVOKABLE QString createPersona(const QString& name);

    /** @brief Get a single persona as QVariantMap. */
    Q_INVOKABLE QVariantMap getPersona(const QString& id) const;

    /** @brief Switch the active persona. */
    Q_INVOKABLE bool selectPersona(const QString& id);

    /** @brief Update persona fields (name, bio). */
    Q_INVOKABLE bool updatePersona(const QString& id, const QString& field, const QString& value);

    /** @brief Delete a persona. */
    Q_INVOKABLE bool deletePersona(const QString& id);

    // ---- Contact CRUD ----------------------------------------------------

    /** @brief List contacts for active persona. */
    Q_INVOKABLE QVariantList listContacts() const;

    /** @brief Create and store a new contact. Returns UID. */
    Q_INVOKABLE QString createContact(const QString& name);

    /** @brief Get a single contact as QVariantMap. */
    Q_INVOKABLE QVariantMap getContact(const QString& id) const;

    /** @brief Update a contact field. */
    Q_INVOKABLE bool updateContact(const QString& id, const QString& field, const QVariant& value);

    /** @brief Delete a contact. */
    Q_INVOKABLE bool deleteContact(const QString& id);

    /** @brief Search contacts by query string. */
    Q_INVOKABLE QVariantList searchContacts(const QString& query) const;

    // ---- Group CRUD ------------------------------------------------------

    /** @brief List all groups. */
    Q_INVOKABLE QVariantList listGroups() const;

    /** @brief Create and store a new group. Returns UID. */
    Q_INVOKABLE QString createGroup(const QString& name);

    /** @brief Get a single group as QVariantMap. */
    Q_INVOKABLE QVariantMap getGroup(const QString& id) const;

    /** @brief Update group fields. */
    Q_INVOKABLE bool updateGroup(const QString& id, const QString& field, const QString& value);

    /** @brief Delete a group. */
    Q_INVOKABLE bool deleteGroup(const QString& id);

    /** @brief Add a contact to a group. */
    Q_INVOKABLE bool addGroupMember(const QString& groupId, const QString& contactId);

    /** @brief Remove a contact from a group. */
    Q_INVOKABLE bool removeGroupMember(const QString& groupId, const QString& contactId);

    /** @brief List group members as [{contactUid, role}, ...]. */
    Q_INVOKABLE QVariantList listGroupMembers(const QString& groupId) const;

    /** @brief Return groups that a given contact belongs to. */
    Q_INVOKABLE QVariantList groupsForContact(const QString& contactId) const;

    // ---- Message CRUD ----------------------------------------------------

    /** @brief List recent messages. */
    Q_INVOKABLE QVariantList listMessages(int limit = 50) const;

    /** @brief Send a message and return its UID. */
    Q_INVOKABLE QString sendMessage(
        const QString& recipientId, const QString& body, const QString& recipientType = QStringLiteral("contact"));

    /** @brief Get a single message as QVariantMap. */
    Q_INVOKABLE QVariantMap getMessage(const QString& id) const;

    /** @brief Delete a message. */
    Q_INVOKABLE bool deleteMessage(const QString& id);

    // ---- Event CRUD ------------------------------------------------------

    /** @brief List all events for the active persona. */
    Q_INVOKABLE QVariantList listEvents() const;

    /** @brief Create and store an event. Returns UID. */
    Q_INVOKABLE QString createEvent(const QString& title, const QString& date);

    /** @brief Get a single event as QVariantMap. */
    Q_INVOKABLE QVariantMap getEvent(const QString& id) const;

    /** @brief Update event fields. */
    Q_INVOKABLE bool updateEvent(const QString& id, const QString& field, const QString& value);

    /** @brief Delete an event. */
    Q_INVOKABLE bool deleteEvent(const QString& id);

    // ---- App lifecycle ---------------------------------------------------

    /** @brief Quit the application. */
    Q_INVOKABLE void exitApp();

    /** @brief Force refresh of all state signals (call after bulk operations). */
    Q_INVOKABLE void refresh();

signals:
    /** @brief Emitted when any profile/persona/container state may have changed. */
    void stateChanged();

    /** @brief Emitted when data changes that QML models should react to. */
    void dataChanged();

    /** @brief Emitted when an error should be shown to the user. */
    void errorOccurred(const QString& message);

    /** @brief Emitted for informational toasts. */
    void infoMessage(const QString& message);

private:
    /** @brief Overwrite buffer with random data before releasing. */
    static void secureWipe(QByteArray& buf);

    /** @brief Convert a protobuf Timestamp to ISO string. */
    static QString tsToIso(const google::protobuf::Timestamp& ts);

    /** @brief Convert a protobuf Timestamp to QDateTime. */
    static QDateTime tsToDateTime(const google::protobuf::Timestamp& ts);

    CoreInterface* m_core = nullptr;
};

#endif // GUIBACKEND_H
