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

#include "GuiBackend.h"
#include "Log.h"

#include <QDateTime>
#include <QRandomGenerator>

GuiBackend::GuiBackend(QObject* parent)
    : QObject(parent)
{
    LOG_D() << __func__;
}

GuiBackend::~GuiBackend()
{
    LOG_D() << __func__;
    //m_core = nullptr;
}

void GuiBackend::setCore(CoreInterface* core)
{
    m_core = core;
    if( m_core )
        emit stateChanged();
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void GuiBackend::secureWipe(QByteArray& buf)
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

QString GuiBackend::tsToIso(const google::protobuf::Timestamp& ts)
{
    return QDateTime::fromMSecsSinceEpoch(ts.seconds() * 1000 + ts.nanos() / 1000000).toString(Qt::ISODate);
}

QDateTime GuiBackend::tsToDateTime(const google::protobuf::Timestamp& ts)
{
    return QDateTime::fromMSecsSinceEpoch(ts.seconds() * 1000 + ts.nanos() / 1000000);
}

// ---------------------------------------------------------------------------
// Property getters
// ---------------------------------------------------------------------------

bool GuiBackend::containerInitialized() const
{
    return m_core && m_core->isContainerInitialized();
}

bool GuiBackend::profileOpen() const
{
    return m_core && m_core->isProfileOpen();
}

QString GuiBackend::profileName() const
{
    if( !m_core || !m_core->isProfileOpen() )
        return {};
    return m_core->getProfile().displayName();
}

QString GuiBackend::profileId() const
{
    return m_core ? m_core->getCurrentProfileId() : QString();
}

QString GuiBackend::activePersonaId() const
{
    return m_core ? m_core->activePersonaId() : QString();
}

QString GuiBackend::activePersonaName() const
{
    return m_core ? m_core->currentPersonaName() : QString();
}

QVariantMap GuiBackend::currentProfile() const
{
    if( !m_core || !m_core->isProfileOpen() )
        return {};

    auto p = m_core->getProfile();
    return {
        {QStringLiteral("uid"), p.uid()},
        {QStringLiteral("displayName"), p.displayName()},
        {QStringLiteral("bio"), p.bio()},
        {QStringLiteral("createdAt"), tsToIso(p.createdAt())},
        {QStringLiteral("updatedAt"), tsToIso(p.updatedAt())},
        {QStringLiteral("hasAvatar"), !p.avatar().isEmpty()},
    };
}

// ---------------------------------------------------------------------------
// Container / Profile lifecycle
// ---------------------------------------------------------------------------

bool GuiBackend::initContainer(int sizeMb)
{
    if( !m_core )
        return false;

    QString path = m_core->getSetting(QStringLiteral("vfs.container.path")).toString();
    if( path.isEmpty() ) {
        emit errorOccurred(tr("Container path not configured"));
        return false;
    }

    const quint64 sizeBytes = static_cast<quint64>(sizeMb) * 1024ULL * 1024ULL;
    bool ok = m_core->createContainer(path, sizeBytes);
    if( ok ) {
        emit infoMessage(tr("Container created (%1 MB)").arg(sizeMb));
        emit stateChanged();
    } else {
        emit errorOccurred(tr("Failed to create container"));
    }
    return ok;
}

bool GuiBackend::openProfile(const QString& password)
{
    if( !m_core )
        return false;

    bool ok = m_core->openProfile(password);
    if( ok ) {
        emit infoMessage(tr("Profile opened: %1").arg(profileName()));
        emit stateChanged();
        emit dataChanged();
    } else {
        emit errorOccurred(tr("Failed to open profile – wrong password or no profile for this key"));
    }
    return ok;
}

bool GuiBackend::createProfile(const QString& password, const QString& displayName)
{
    if( !m_core )
        return false;

    bool ok = m_core->createProfile(password, displayName);
    if( ok ) {
        emit infoMessage(tr("Profile created: %1").arg(displayName));
        emit stateChanged();
        emit dataChanged();
    } else {
        emit errorOccurred(tr("Failed to create profile"));
    }
    return ok;
}

void GuiBackend::closeProfile()
{
    if( !m_core )
        return;
    m_core->closeProfile();
    emit stateChanged();
    emit dataChanged();
}

bool GuiBackend::deleteProfile()
{
    if( !m_core )
        return false;

    bool ok = m_core->deleteCurrentProfile();
    if( ok ) {
        emit infoMessage(tr("Profile securely deleted"));
        emit stateChanged();
        emit dataChanged();
    } else {
        emit errorOccurred(tr("Failed to delete profile"));
    }
    return ok;
}

QString GuiBackend::exportProfile()
{
    if( !m_core )
        return {};
    QByteArray data = m_core->exportProfile();
    QString result = QString::fromUtf8(data);
    secureWipe(data);
    return result;
}

bool GuiBackend::importProfile(const QString& data, const QString& vfsPassword)
{
    if( !m_core )
        return false;
    bool ok = m_core->importProfile(data.toUtf8(), QString(), vfsPassword);
    if( ok ) {
        emit infoMessage(tr("Profile imported successfully"));
        emit stateChanged();
        emit dataChanged();
    }
    return ok;
}

// ---------------------------------------------------------------------------
// Settings
// ---------------------------------------------------------------------------

QVariant GuiBackend::getSetting(const QString& key) const
{
    return m_core ? m_core->getSetting(key) : QVariant();
}

bool GuiBackend::setSetting(const QString& key, const QVariant& value)
{
    return m_core ? m_core->setSetting(key, value) : false;
}

QStringList GuiBackend::listSettings() const
{
    return m_core ? m_core->listSettings() : QStringList();
}

// ---------------------------------------------------------------------------
// Profile parameters
// ---------------------------------------------------------------------------

QVariantList GuiBackend::listParams() const
{
    QVariantList result;
    if( !m_core )
        return result;
    auto params = m_core->listParams();
    for( const auto& p : params )
        result.append(QVariantMap{{QStringLiteral("key"), p.key()}, {QStringLiteral("value"), p.value()}});
    return result;
}

QString GuiBackend::getParam(const QString& key) const
{
    if( !m_core )
        return {};
    return m_core->getParam(key).value();
}

bool GuiBackend::setParam(const QString& key, const QString& value)
{
    if( !m_core )
        return false;
    auto pp = m_core->createParam(key);
    pp.setValue(value);
    bool ok = m_core->storeParam(pp);
    if( ok )
        emit dataChanged();
    return ok;
}

bool GuiBackend::deleteParam(const QString& key)
{
    if( !m_core )
        return false;
    bool ok = m_core->deleteParam(key);
    if( ok )
        emit dataChanged();
    return ok;
}

// ---------------------------------------------------------------------------
// Persona
// ---------------------------------------------------------------------------

QVariantList GuiBackend::listPersonas() const
{
    QVariantList result;
    if( !m_core )
        return result;
    auto personas = m_core->listPersonas();
    for( const auto& p : personas ) {
        result.append(QVariantMap{
            {QStringLiteral("uid"), p.uid()},
            {QStringLiteral("displayName"), p.displayName()},
            {QStringLiteral("bio"), p.bio()},
            {QStringLiteral("isDefault"), p.isDefault()},
            {QStringLiteral("isActive"), p.uid() == m_core->activePersonaId()},
            {QStringLiteral("hasAvatar"), !p.avatar().isEmpty()},
            {QStringLiteral("createdAt"), tsToIso(p.createdAt())},
        });
    }
    return result;
}

QString GuiBackend::createPersona(const QString& name)
{
    if( !m_core )
        return {};
    auto p = m_core->createPersona(name);
    if( p.uid().isEmpty() || !m_core->storePersona(p) )
        return {};
    emit dataChanged();
    return p.uid();
}

QVariantMap GuiBackend::getPersona(const QString& id) const
{
    if( !m_core )
        return {};
    auto p = m_core->getPersona(id);
    if( p.uid().isEmpty() )
        return {};
    return {
        {QStringLiteral("uid"), p.uid()},
        {QStringLiteral("displayName"), p.displayName()},
        {QStringLiteral("bio"), p.bio()},
        {QStringLiteral("isDefault"), p.isDefault()},
        {QStringLiteral("hasAvatar"), !p.avatar().isEmpty()},
        {QStringLiteral("createdAt"), tsToIso(p.createdAt())},
    };
}

bool GuiBackend::selectPersona(const QString& id)
{
    if( !m_core )
        return false;
    bool ok = m_core->setActivePersona(id);
    if( ok ) {
        emit stateChanged();
        emit dataChanged();
    }
    return ok;
}

bool GuiBackend::updatePersona(const QString& id, const QString& field, const QString& value)
{
    if( !m_core )
        return false;
    auto p = m_core->getPersona(id);
    if( p.uid().isEmpty() )
        return false;

    if( field == QLatin1String("name") )
        p.setDisplayName(value);
    else if( field == QLatin1String("bio") )
        p.setBio(value);
    else
        return false;

    bool ok = m_core->storePersona(p);
    if( ok )
        emit dataChanged();
    return ok;
}

bool GuiBackend::deletePersona(const QString& id)
{
    if( !m_core )
        return false;
    bool ok = m_core->deletePersona(id);
    if( ok )
        emit dataChanged();
    return ok;
}

// ---------------------------------------------------------------------------
// Contact
// ---------------------------------------------------------------------------

QVariantList GuiBackend::listContacts() const
{
    QVariantList result;
    if( !m_core )
        return result;
    auto contacts = m_core->listContacts();
    for( const auto& c : contacts ) {
        QVariantMap m{
            {QStringLiteral("uid"), c.uid()},
            {QStringLiteral("displayName"), c.displayName()},
            {QStringLiteral("trustLevel"), static_cast<int>(c.trustLevel())},
            {QStringLiteral("notes"), c.notes()},
            {QStringLiteral("hasAvatar"), !c.avatar().isEmpty()},
            {QStringLiteral("relationshipType"), c.relationshipType()},
            {QStringLiteral("parentContactUid"), c.parentContactUid()},
        };
        if( c.hasBirthday() )
            m[QStringLiteral("birthday")] = tsToIso(c.birthday());
        if( c.hasDeathday() )
            m[QStringLiteral("deathday")] = tsToIso(c.deathday());
        result.append(m);
    }
    return result;
}

QString GuiBackend::createContact(const QString& name)
{
    if( !m_core )
        return {};
    auto c = m_core->createContact(name);
    if( c.uid().isEmpty() || !m_core->storeContact(c) )
        return {};
    emit dataChanged();
    return c.uid();
}

QVariantMap GuiBackend::getContact(const QString& id) const
{
    if( !m_core )
        return {};
    auto c = m_core->getContact(id);
    if( c.uid().isEmpty() )
        return {};
    QVariantMap m{
        {QStringLiteral("uid"), c.uid()},
        {QStringLiteral("displayName"), c.displayName()},
        {QStringLiteral("trustLevel"), static_cast<int>(c.trustLevel())},
        {QStringLiteral("notes"), c.notes()},
        {QStringLiteral("hasAvatar"), !c.avatar().isEmpty()},
        {QStringLiteral("relationshipType"), c.relationshipType()},
        {QStringLiteral("parentContactUid"), c.parentContactUid()},
    };
    if( c.hasBirthday() )
        m[QStringLiteral("birthday")] = tsToIso(c.birthday());
    if( c.hasDeathday() )
        m[QStringLiteral("deathday")] = tsToIso(c.deathday());
    return m;
}

bool GuiBackend::updateContact(const QString& id, const QString& field, const QVariant& value)
{
    if( !m_core )
        return false;
    auto c = m_core->getContact(id);
    if( c.uid().isEmpty() )
        return false;

    if( field == QLatin1String("name") )
        c.setDisplayName(value.toString());
    else if( field == QLatin1String("notes") )
        c.setNotes(value.toString());
    else if( field == QLatin1String("trust") )
        c.setTrustLevel(value.toInt());
    else if( field == QLatin1String("relationshipType") )
        c.setRelationshipType(value.toString());
    else if( field == QLatin1String("parentContactUid") )
        c.setParentContactUid(value.toString());
    else
        return false;

    bool ok = m_core->storeContact(c);
    if( ok )
        emit dataChanged();
    return ok;
}

bool GuiBackend::deleteContact(const QString& id)
{
    if( !m_core )
        return false;
    bool ok = m_core->deleteContact(id);
    if( ok )
        emit dataChanged();
    return ok;
}

QVariantList GuiBackend::searchContacts(const QString& query) const
{
    QVariantList result;
    if( !m_core )
        return result;
    auto contacts = m_core->searchContacts(query);
    for( const auto& c : contacts ) {
        result.append(QVariantMap{
            {QStringLiteral("uid"), c.uid()},
            {QStringLiteral("displayName"), c.displayName()},
            {QStringLiteral("trustLevel"), static_cast<int>(c.trustLevel())},
        });
    }
    return result;
}

// ---------------------------------------------------------------------------
// Group
// ---------------------------------------------------------------------------

QVariantList GuiBackend::listGroups() const
{
    QVariantList result;
    if( !m_core )
        return result;
    auto groups = m_core->listGroups();
    for( const auto& g : groups ) {
        result.append(QVariantMap{
            {QStringLiteral("uid"), g.uid()},
            {QStringLiteral("name"), g.name()},
            {QStringLiteral("description"), g.description()},
            {QStringLiteral("color"), g.color()},
        });
    }
    return result;
}

QString GuiBackend::createGroup(const QString& name)
{
    if( !m_core )
        return {};
    auto g = m_core->createGroup(name);
    if( g.uid().isEmpty() || !m_core->storeGroup(g) )
        return {};
    emit dataChanged();
    return g.uid();
}

QVariantMap GuiBackend::getGroup(const QString& id) const
{
    if( !m_core )
        return {};
    auto g = m_core->getGroup(id);
    if( g.uid().isEmpty() )
        return {};
    return {
        {QStringLiteral("uid"), g.uid()},
        {QStringLiteral("name"), g.name()},
        {QStringLiteral("description"), g.description()},
        {QStringLiteral("color"), g.color()},
    };
}

bool GuiBackend::updateGroup(const QString& id, const QString& field, const QString& value)
{
    if( !m_core )
        return false;
    auto g = m_core->getGroup(id);
    if( g.uid().isEmpty() )
        return false;

    if( field == QLatin1String("name") )
        g.setName(value);
    else if( field == QLatin1String("description") )
        g.setDescription(value);
    else if( field == QLatin1String("color") )
        g.setColor(value);
    else
        return false;

    bool ok = m_core->storeGroup(g);
    if( ok )
        emit dataChanged();
    return ok;
}

bool GuiBackend::deleteGroup(const QString& id)
{
    if( !m_core )
        return false;
    bool ok = m_core->deleteGroup(id);
    if( ok )
        emit dataChanged();
    return ok;
}

bool GuiBackend::addGroupMember(const QString& groupId, const QString& contactId)
{
    if( !m_core )
        return false;
    bool ok = m_core->addGroupMember(groupId, contactId);
    if( ok )
        emit dataChanged();
    return ok;
}

bool GuiBackend::removeGroupMember(const QString& groupId, const QString& contactId)
{
    if( !m_core )
        return false;
    bool ok = m_core->removeGroupMember(groupId, contactId);
    if( ok )
        emit dataChanged();
    return ok;
}

QVariantList GuiBackend::listGroupMembers(const QString& groupId) const
{
    QVariantList result;
    if( !m_core )
        return result;
    auto members = m_core->listGroupMembers(groupId);
    for( const auto& gm : members ) {
        result.append(QVariantMap{
            {QStringLiteral("contactUid"), gm.contactUid()},
            {QStringLiteral("role"), gm.role()},
        });
    }
    return result;
}

QVariantList GuiBackend::groupsForContact(const QString& contactId) const
{
    QVariantList result;
    if( !m_core )
        return result;
    auto groups = m_core->listGroups();
    for( const auto& g : groups ) {
        auto members = m_core->listGroupMembers(g.uid());
        for( const auto& gm : members ) {
            if( gm.contactUid() == contactId ) {
                result.append(QVariantMap{
                    {QStringLiteral("uid"), g.uid()},
                    {QStringLiteral("name"), g.name()},
                    {QStringLiteral("color"), g.color()},
                });
                break;
            }
        }
    }
    return result;
}

// ---------------------------------------------------------------------------
// Message
// ---------------------------------------------------------------------------

QVariantList GuiBackend::listMessages(int limit) const
{
    QVariantList result;
    if( !m_core )
        return result;
    auto messages = m_core->listMessages(limit);
    for( const auto& msg : messages ) {
        result.append(QVariantMap{
            {QStringLiteral("uid"), msg.uid()},
            {QStringLiteral("threadUid"), msg.threadUid()},
            {QStringLiteral("senderContactUid"), msg.senderContactUid()},
            {QStringLiteral("recipientType"), msg.recipientType()},
            {QStringLiteral("recipientUid"), msg.recipientUid()},
            {QStringLiteral("body"), msg.body()},
            {QStringLiteral("contentType"), msg.contentType()},
            {QStringLiteral("isRead"), msg.isRead()},
            {QStringLiteral("createdAt"), tsToIso(msg.createdAt())},
        });
    }
    return result;
}

QString GuiBackend::sendMessage(const QString& recipientId, const QString& body, const QString& recipientType)
{
    if( !m_core )
        return {};
    auto msg = m_core->createMessage(recipientId, body, recipientType);
    if( msg.uid().isEmpty() || !m_core->storeMessage(msg) )
        return {};
    emit dataChanged();
    return msg.uid();
}

QVariantMap GuiBackend::getMessage(const QString& id) const
{
    if( !m_core )
        return {};
    auto msg = m_core->getMessage(id);
    if( msg.uid().isEmpty() )
        return {};
    return {
        {QStringLiteral("uid"), msg.uid()},
        {QStringLiteral("threadUid"), msg.threadUid()},
        {QStringLiteral("senderContactUid"), msg.senderContactUid()},
        {QStringLiteral("recipientType"), msg.recipientType()},
        {QStringLiteral("recipientUid"), msg.recipientUid()},
        {QStringLiteral("body"), msg.body()},
        {QStringLiteral("contentType"), msg.contentType()},
        {QStringLiteral("isRead"), msg.isRead()},
        {QStringLiteral("createdAt"), tsToIso(msg.createdAt())},
    };
}

bool GuiBackend::deleteMessage(const QString& id)
{
    if( !m_core )
        return false;
    bool ok = m_core->deleteMessage(id);
    if( ok )
        emit dataChanged();
    return ok;
}

// ---------------------------------------------------------------------------
// Event
// ---------------------------------------------------------------------------

QVariantList GuiBackend::listEvents() const
{
    QVariantList result;
    if( !m_core )
        return result;
    auto events = m_core->listEvents();
    for( const auto& e : events ) {
        QVariantMap m{
            {QStringLiteral("uid"), e.uid()},
            {QStringLiteral("title"), e.title()},
            {QStringLiteral("description"), e.description()},
            {QStringLiteral("location"), e.location()},
            {QStringLiteral("eventType"), e.eventType()},
            {QStringLiteral("color"), e.color()},
            {QStringLiteral("category"), e.category()},
        };
        if( e.hasStarted() ) {
            m[QStringLiteral("started")] = tsToIso(e.started());
            m[QStringLiteral("startedMs")] = static_cast<double>(tsToDateTime(e.started()).toMSecsSinceEpoch());
        }
        if( e.hasEnded() ) {
            m[QStringLiteral("ended")] = tsToIso(e.ended());
            m[QStringLiteral("endedMs")] = static_cast<double>(tsToDateTime(e.ended()).toMSecsSinceEpoch());
        }
        result.append(m);
    }
    return result;
}

QString GuiBackend::createEvent(const QString& title, const QString& date)
{
    if( !m_core )
        return {};
    auto e = m_core->createEvent(title, date);
    if( e.uid().isEmpty() || !m_core->storeEvent(e) )
        return {};
    emit dataChanged();
    return e.uid();
}

QVariantMap GuiBackend::getEvent(const QString& id) const
{
    if( !m_core )
        return {};
    auto e = m_core->getEvent(id);
    if( e.uid().isEmpty() )
        return {};
    QVariantMap m{
        {QStringLiteral("uid"), e.uid()},
        {QStringLiteral("title"), e.title()},
        {QStringLiteral("description"), e.description()},
        {QStringLiteral("location"), e.location()},
        {QStringLiteral("eventType"), e.eventType()},
        {QStringLiteral("color"), e.color()},
        {QStringLiteral("category"), e.category()},
    };
    if( e.hasStarted() )
        m[QStringLiteral("started")] = tsToIso(e.started());
    if( e.hasEnded() )
        m[QStringLiteral("ended")] = tsToIso(e.ended());
    return m;
}

bool GuiBackend::updateEvent(const QString& id, const QString& field, const QString& value)
{
    if( !m_core )
        return false;
    auto e = m_core->getEvent(id);
    if( e.uid().isEmpty() )
        return false;

    if( field == QLatin1String("title") )
        e.setTitle(value);
    else if( field == QLatin1String("description") )
        e.setDescription(value);
    else if( field == QLatin1String("location") )
        e.setLocation(value);
    else if( field == QLatin1String("color") )
        e.setColor(value);
    else if( field == QLatin1String("category") )
        e.setCategory(value);
    else if( field == QLatin1String("eventType") )
        e.setEventType(value);
    else
        return false;

    bool ok = m_core->storeEvent(e);
    if( ok )
        emit dataChanged();
    return ok;
}

bool GuiBackend::deleteEvent(const QString& id)
{
    if( !m_core )
        return false;
    bool ok = m_core->deleteEvent(id);
    if( ok )
        emit dataChanged();
    return ok;
}

// ---------------------------------------------------------------------------
// App
// ---------------------------------------------------------------------------

void GuiBackend::exitApp()
{
    if( m_core )
        m_core->exit();
}

void GuiBackend::refresh()
{
    emit stateChanged();
    emit dataChanged();
}
