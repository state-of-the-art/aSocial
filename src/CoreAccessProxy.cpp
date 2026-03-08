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

#include "CoreAccessProxy.h"

#include <QLoggingCategory>
#include <QRandomGenerator>

Q_LOGGING_CATEGORY(Cap, "CoreAccessProxy")

CoreAccessProxy::CoreAccessProxy(
    CoreInterface* real, PluginPermissions granted, const QString& pluginName, QObject* parent)
    : QObject(parent)
    , m_real(real)
    , m_granted(expandImpliedPermissions(granted))
    , m_pluginName(pluginName)
{
    // Forward profileChanged from the real Core (which is a QObject)
    auto* realObj = dynamic_cast<QObject*>(real);
    if( realObj )
        connect(realObj, SIGNAL(profileChanged()), this, SIGNAL(profileChanged()));
}

CoreAccessProxy::~CoreAccessProxy() = default;

// ---------------------------------------------------------------------------
// Permission gate
// ---------------------------------------------------------------------------

bool CoreAccessProxy::check(PluginPermission perm, const char* method) const
{
    if( m_granted.testFlag(perm) )
        return true;
    qCWarning(Cap) << "Plugin" << m_pluginName << "denied:" << method
                   << "(missing permission:" << permissionNames(PluginPermissions(perm)).join(", ") << ")";
    return false;
}

// ---------------------------------------------------------------------------
// Plugin accessors — PluginAccess
// ---------------------------------------------------------------------------

DBKVPluginInterface* CoreAccessProxy::getDBKV() const
{
    if( !check(PluginPermission::PluginAccess, __func__) )
        return nullptr;
    return m_real->getDBKV();
}

DBKVPluginInterface* CoreAccessProxy::getDBKVProfile() const
{
    if( !check(PluginPermission::PluginAccess, __func__) )
        return nullptr;
    return m_real->getDBKVProfile();
}

VFSPluginInterface* CoreAccessProxy::getVFS() const
{
    if( !check(PluginPermission::PluginAccess, __func__) )
        return nullptr;
    return m_real->getVFS();
}

// ---------------------------------------------------------------------------
// Container lifecycle — ContainerRead / ContainerWrite
// ---------------------------------------------------------------------------

bool CoreAccessProxy::createContainer(const QString& path, quint64 maxSizeBytes)
{
    if( !check(PluginPermission::ContainerWrite, __func__) )
        return false;
    return m_real->createContainer(path, maxSizeBytes);
}

bool CoreAccessProxy::isContainerInitialized() const
{
    if( !check(PluginPermission::ContainerRead, __func__) )
        return false;
    return m_real->isContainerInitialized();
}

QString CoreAccessProxy::containerPath() const
{
    if( !check(PluginPermission::ContainerRead, __func__) )
        return {};
    return m_real->containerPath();
}

// ---------------------------------------------------------------------------
// Profile lifecycle — ProfileRead / ProfileWrite / ProfileDelete
// ---------------------------------------------------------------------------

bool CoreAccessProxy::openProfile(const QString& password)
{
    if( !check(PluginPermission::ProfileWrite, __func__) )
        return false;
    return m_real->openProfile(password);
}

bool CoreAccessProxy::createProfile(const QString& password, const QString& displayName)
{
    if( !check(PluginPermission::ProfileWrite, __func__) )
        return false;
    return m_real->createProfile(password, displayName);
}

void CoreAccessProxy::closeProfile()
{
    if( !check(PluginPermission::ProfileWrite, __func__) )
        return;
    m_real->closeProfile();
}

bool CoreAccessProxy::isProfileOpen() const
{
    if( !check(PluginPermission::ProfileRead, __func__) )
        return false;
    return m_real->isProfileOpen();
}

bool CoreAccessProxy::deleteCurrentProfile(std::function<void(int)> progress)
{
    if( !check(PluginPermission::ProfileDelete, __func__) )
        return false;
    return m_real->deleteCurrentProfile(std::move(progress));
}

// ---------------------------------------------------------------------------
// Profile data — ProfileRead / ProfileWrite
// ---------------------------------------------------------------------------

asocial::v1::Profile CoreAccessProxy::getProfile() const
{
    if( !check(PluginPermission::ProfileRead, __func__) )
        return {};
    return m_real->getProfile();
}

bool CoreAccessProxy::storeProfile(const asocial::v1::Profile& profile)
{
    if( !check(PluginPermission::ProfileWrite, __func__) )
        return false;
    return m_real->storeProfile(profile);
}

QString CoreAccessProxy::getCurrentProfileId() const
{
    if( !check(PluginPermission::ProfileRead, __func__) )
        return {};
    return m_real->getCurrentProfileId();
}

QString CoreAccessProxy::currentPersonaName() const
{
    if( !check(PluginPermission::ProfileRead, __func__) )
        return {};
    return m_real->currentPersonaName();
}

QList<asocial::v1::ProfileParameter> CoreAccessProxy::listParams() const
{
    if( !check(PluginPermission::DataRead, __func__) )
        return {};
    return m_real->listParams();
}

asocial::v1::ProfileParameter CoreAccessProxy::createParam(const QString& paramKey)
{
    if( !check(PluginPermission::DataWrite, __func__) )
        return {};
    return m_real->createParam(paramKey);
}

asocial::v1::ProfileParameter CoreAccessProxy::getParam(const QString& paramKey) const
{
    if( !check(PluginPermission::DataRead, __func__) )
        return {};
    return m_real->getParam(paramKey);
}

bool CoreAccessProxy::storeParam(const asocial::v1::ProfileParameter& param)
{
    if( !check(PluginPermission::DataWrite, __func__) )
        return false;
    return m_real->storeParam(param);
}

bool CoreAccessProxy::deleteParam(const QString& paramKey)
{
    if( !check(PluginPermission::DataWrite, __func__) )
        return false;
    return m_real->deleteParam(paramKey);
}

// ---------------------------------------------------------------------------
// Export / Import — ProfileExport / ProfileImport
// ---------------------------------------------------------------------------

QByteArray CoreAccessProxy::exportProfile(const QString& encryptionPassword)
{
    if( !check(PluginPermission::ProfileExport, __func__) )
        return {};
    QByteArray result = m_real->exportProfile(encryptionPassword);
    return result;
}

bool CoreAccessProxy::importProfile(const QByteArray& data, const QString& decryptionPassword, const QString& vfsPassword)
{
    if( !check(PluginPermission::ProfileImport, __func__) )
        return false;
    return m_real->importProfile(data, decryptionPassword, vfsPassword);
}

// ---------------------------------------------------------------------------
// Settings — SettingsRead / SettingsWrite
// ---------------------------------------------------------------------------

QVariant CoreAccessProxy::getSetting(const QString& key) const
{
    if( !check(PluginPermission::SettingsRead, __func__) )
        return {};
    return m_real->getSetting(key);
}

bool CoreAccessProxy::setSetting(const QString& key, const QVariant& value)
{
    if( !check(PluginPermission::SettingsWrite, __func__) )
        return false;
    return m_real->setSetting(key, value);
}

QStringList CoreAccessProxy::listSettings() const
{
    if( !check(PluginPermission::SettingsRead, __func__) )
        return {};
    return m_real->listSettings();
}

// ---------------------------------------------------------------------------
// Active persona — DataRead / DataWrite
// ---------------------------------------------------------------------------

bool CoreAccessProxy::setActivePersona(const QString& personaId)
{
    if( !check(PluginPermission::DataWrite, __func__) )
        return false;
    return m_real->setActivePersona(personaId);
}

QString CoreAccessProxy::activePersonaId() const
{
    if( !check(PluginPermission::DataRead, __func__) )
        return {};
    return m_real->activePersonaId();
}

// ---------------------------------------------------------------------------
// Persona CRUD — DataRead / DataWrite
// ---------------------------------------------------------------------------

QList<asocial::v1::Persona> CoreAccessProxy::listPersonas() const
{
    if( !check(PluginPermission::DataRead, __func__) )
        return {};
    return m_real->listPersonas();
}

asocial::v1::Persona CoreAccessProxy::createPersona(const QString& displayName)
{
    if( !check(PluginPermission::DataWrite, __func__) )
        return {};
    return m_real->createPersona(displayName);
}

asocial::v1::Persona CoreAccessProxy::getPersona(const QString& personaId) const
{
    if( !check(PluginPermission::DataRead, __func__) )
        return {};
    return m_real->getPersona(personaId);
}

bool CoreAccessProxy::storePersona(const asocial::v1::Persona& persona)
{
    if( !check(PluginPermission::DataWrite, __func__) )
        return false;
    return m_real->storePersona(persona);
}

bool CoreAccessProxy::deletePersona(const QString& personaId)
{
    if( !check(PluginPermission::DataWrite, __func__) )
        return false;
    return m_real->deletePersona(personaId);
}

// ---------------------------------------------------------------------------
// Contact CRUD — DataRead / DataWrite
// ---------------------------------------------------------------------------

QList<asocial::v1::Contact> CoreAccessProxy::listContacts() const
{
    if( !check(PluginPermission::DataRead, __func__) )
        return {};
    return m_real->listContacts();
}

asocial::v1::Contact CoreAccessProxy::createContact(const QString& displayName)
{
    if( !check(PluginPermission::DataWrite, __func__) )
        return {};
    return m_real->createContact(displayName);
}

asocial::v1::Contact CoreAccessProxy::getContact(const QString& contactId) const
{
    if( !check(PluginPermission::DataRead, __func__) )
        return {};
    return m_real->getContact(contactId);
}

bool CoreAccessProxy::storeContact(const asocial::v1::Contact& contact)
{
    if( !check(PluginPermission::DataWrite, __func__) )
        return false;
    return m_real->storeContact(contact);
}

bool CoreAccessProxy::deleteContact(const QString& contactId)
{
    if( !check(PluginPermission::DataWrite, __func__) )
        return false;
    return m_real->deleteContact(contactId);
}

QList<asocial::v1::Contact> CoreAccessProxy::searchContacts(const QString& query) const
{
    if( !check(PluginPermission::DataRead, __func__) )
        return {};
    return m_real->searchContacts(query);
}

// ---------------------------------------------------------------------------
// Group CRUD — DataRead / DataWrite
// ---------------------------------------------------------------------------

QList<asocial::v1::Group> CoreAccessProxy::listGroups() const
{
    if( !check(PluginPermission::DataRead, __func__) )
        return {};
    return m_real->listGroups();
}

asocial::v1::Group CoreAccessProxy::createGroup(const QString& name)
{
    if( !check(PluginPermission::DataWrite, __func__) )
        return {};
    return m_real->createGroup(name);
}

asocial::v1::Group CoreAccessProxy::getGroup(const QString& groupId) const
{
    if( !check(PluginPermission::DataRead, __func__) )
        return {};
    return m_real->getGroup(groupId);
}

bool CoreAccessProxy::storeGroup(const asocial::v1::Group& group)
{
    if( !check(PluginPermission::DataWrite, __func__) )
        return false;
    return m_real->storeGroup(group);
}

bool CoreAccessProxy::deleteGroup(const QString& groupId)
{
    if( !check(PluginPermission::DataWrite, __func__) )
        return false;
    return m_real->deleteGroup(groupId);
}

bool CoreAccessProxy::addGroupMember(const QString& groupId, const QString& contactId)
{
    if( !check(PluginPermission::DataWrite, __func__) )
        return false;
    return m_real->addGroupMember(groupId, contactId);
}

bool CoreAccessProxy::removeGroupMember(const QString& groupId, const QString& contactId)
{
    if( !check(PluginPermission::DataWrite, __func__) )
        return false;
    return m_real->removeGroupMember(groupId, contactId);
}

QList<asocial::v1::GroupMember> CoreAccessProxy::listGroupMembers(const QString& groupId) const
{
    if( !check(PluginPermission::DataRead, __func__) )
        return {};
    return m_real->listGroupMembers(groupId);
}

// ---------------------------------------------------------------------------
// Message CRUD — DataRead / DataWrite
// ---------------------------------------------------------------------------

QList<asocial::v1::Message> CoreAccessProxy::listMessages(int limit) const
{
    if( !check(PluginPermission::DataRead, __func__) )
        return {};
    return m_real->listMessages(limit);
}

asocial::v1::Message CoreAccessProxy::createMessage(
    const QString& recipientId, const QString& body, const QString& recipientType)
{
    if( !check(PluginPermission::DataWrite, __func__) )
        return {};
    return m_real->createMessage(recipientId, body, recipientType);
}

asocial::v1::Message CoreAccessProxy::getMessage(const QString& messageId) const
{
    if( !check(PluginPermission::DataRead, __func__) )
        return {};
    return m_real->getMessage(messageId);
}

bool CoreAccessProxy::storeMessage(const asocial::v1::Message& message)
{
    if( !check(PluginPermission::DataWrite, __func__) )
        return false;
    return m_real->storeMessage(message);
}

bool CoreAccessProxy::deleteMessage(const QString& messageId)
{
    if( !check(PluginPermission::DataWrite, __func__) )
        return false;
    return m_real->deleteMessage(messageId);
}

// ---------------------------------------------------------------------------
// Event CRUD — DataRead / DataWrite
// ---------------------------------------------------------------------------

QList<asocial::v1::Event> CoreAccessProxy::listEvents() const
{
    if( !check(PluginPermission::DataRead, __func__) )
        return {};
    return m_real->listEvents();
}

asocial::v1::Event CoreAccessProxy::createEvent(const QString& title, const QString& date)
{
    if( !check(PluginPermission::DataWrite, __func__) )
        return {};
    return m_real->createEvent(title, date);
}

asocial::v1::Event CoreAccessProxy::getEvent(const QString& eventId) const
{
    if( !check(PluginPermission::DataRead, __func__) )
        return {};
    return m_real->getEvent(eventId);
}

bool CoreAccessProxy::storeEvent(const asocial::v1::Event& event)
{
    if( !check(PluginPermission::DataWrite, __func__) )
        return false;
    return m_real->storeEvent(event);
}

bool CoreAccessProxy::deleteEvent(const QString& eventId)
{
    if( !check(PluginPermission::DataWrite, __func__) )
        return false;
    return m_real->deleteEvent(eventId);
}

// ---------------------------------------------------------------------------
// App lifecycle — AppLifecycle
// ---------------------------------------------------------------------------

void CoreAccessProxy::exit()
{
    if( !check(PluginPermission::AppLifecycle, __func__) )
        return;
    m_real->exit();
}

// ---------------------------------------------------------------------------
// Security
// ---------------------------------------------------------------------------

void CoreAccessProxy::secureWipe(QByteArray& buf)
{
    if( buf.isEmpty() )
        return;

    auto* rng = QRandomGenerator::global();
    auto* data = reinterpret_cast<quint32*>(buf.data());
    const qsizetype words = buf.size() / static_cast<qsizetype>(sizeof(quint32));
    for( qsizetype i = 0; i < words; ++i )
        data[i] = rng->generate();

    const qsizetype tail = buf.size() % static_cast<qsizetype>(sizeof(quint32));
    for( qsizetype i = buf.size() - tail; i < buf.size(); ++i )
        buf[i] = static_cast<char>(rng->generate() & 0xFF);

    buf.clear();
}
