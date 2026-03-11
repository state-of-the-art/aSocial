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

#include "core.h"
#include "CoreInterface.h"
#include "plugins.h"
#include "settings.h"

#include "Log.h"
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QRandomGenerator>
#include <QUuid>

#include <QtProtobuf/QProtobufJsonSerializer>

#include "asocial/v1/contact.qpb.h"
#include "asocial/v1/event.qpb.h"
#include "asocial/v1/group.qpb.h"
#include "asocial/v1/message.qpb.h"
#include "asocial/v1/profile.qpb.h"
#include "asocial/v1/profile_param.qpb.h"

// ---- Key-scheme constants --------------------------------------------------
// Hierarchical prefix design for efficient range scans in RocksDB:
//   p                                       -> Profile
//   a/<persona_uid>                         -> Persona
//   c/<persona_uid>/<contact_uid>           -> Contact
//   g/<persona_uid>/<group_uid>             -> Group
//   gm/<group_uid>/<contact_uid>            -> GroupMember
//   m/<persona_uid>/<created_ms>/<msg_uid>  -> Message (time-ordered)
//   e/<persona_uid>/<event_uid>             -> Event
//   pp/<key>                                -> ProfileParam
namespace KV {
static const QString PROFILE = QStringLiteral("p");
static const QString PERSONA_PFX = QStringLiteral("a/");
static const QString CONTACT_PFX = QStringLiteral("c/");
static const QString GROUP_PFX = QStringLiteral("g/");
static const QString GROUP_MEMBER_PFX = QStringLiteral("gm/");
static const QString MESSAGE_PFX = QStringLiteral("m/");
static const QString EVENT_PFX = QStringLiteral("e/");
static const QString PARAM_PFX = QStringLiteral("pp/");

inline QString paramKey(const QString& paramKey)
{
    return PERSONA_PFX + paramKey;
}
inline QString personaKey(const QString& uid)
{
    return PERSONA_PFX + uid;
}
inline QString contactKey(const QString& personaUid, const QString& uid)
{
    return CONTACT_PFX + personaUid + '/' + uid;
}
inline QString contactPrefix(const QString& personaUid)
{
    return CONTACT_PFX + personaUid + '/';
}
inline QString groupKey(const QString& personaUid, const QString& uid)
{
    return GROUP_PFX + personaUid + '/' + uid;
}
inline QString groupPrefix(const QString& personaUid)
{
    return GROUP_PFX + personaUid + '/';
}
inline QString groupMemberKey(const QString& groupUid, const QString& contactUid)
{
    return GROUP_MEMBER_PFX + groupUid + '/' + contactUid;
}
inline QString groupMemberPrefix(const QString& groupUid)
{
    return GROUP_MEMBER_PFX + groupUid + '/';
}
inline QString messageKey(const QString& personaUid, qint64 createdMs, const QString& uid)
{
    return MESSAGE_PFX + personaUid + '/' + QString::number(createdMs).rightJustified(16, '0') + '/' + uid;
}
inline QString messagePrefix(const QString& personaUid)
{
    return MESSAGE_PFX + personaUid + '/';
}
inline QString eventKey(const QString& personaUid, const QString& uid)
{
    return EVENT_PFX + personaUid + '/' + uid;
}
inline QString eventPrefix(const QString& personaUid)
{
    return EVENT_PFX + personaUid + '/';
}
inline QString settingKey(const QString& key)
{
    return PARAM_PFX + key;
}
} // namespace KV

// ---- Helpers for Timestamp <-> QDateTime -----------------------------------
namespace {

google::protobuf::Timestamp toTimestamp(const QDateTime& dt)
{
    google::protobuf::Timestamp ts;
    ts.setSeconds(dt.toSecsSinceEpoch());
    ts.setNanos(dt.time().msec() * 1000000);
    return ts;
}

QDateTime fromTimestamp(const google::protobuf::Timestamp& ts)
{
    return QDateTime::fromMSecsSinceEpoch(ts.seconds() * 1000 + ts.nanos() / 1000000);
}

QString timestampToIso(const google::protobuf::Timestamp& ts)
{
    return fromTimestamp(ts).toString(Qt::ISODate);
}

/** @brief Extract epoch milliseconds from a Timestamp for key composition. */
qint64 timestampToMsEpoch(const google::protobuf::Timestamp& ts)
{
    return ts.seconds() * 1000 + ts.nanos() / 1000000;
}

} // namespace

// ---------------------------------------------------------------------------
Core* Core::s_pInstance = nullptr;

Core::Core(QObject* parent)
    : QObject(parent)
{
    LOG_D() << "Core singleton created";
}

Core::~Core()
{
    closeProfile();
    LOG_D() << "Core singleton destroyed";
}

// ---------------------------------------------------------------------------
// Utility
// ---------------------------------------------------------------------------

QString Core::newId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void Core::secureWipe(QByteArray& buf)
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

void Core::flushProfile()
{
    if( m_dbkvProfile && m_dbkvProfile->isDatabaseOpen() )
        m_dbkvProfile->flushDatabase();
}

// ---------------------------------------------------------------------------
// Plugin wiring
// ---------------------------------------------------------------------------

void Core::setDBKVPlugin(const QString& pluginName)
{
    Settings::I()->setting("plugins.dbkv.json.active", true);
    Plugins::I()->settingActivePlugin("plugins.dbkv.json.active", "dbkv-json");
    Plugins::I()->activateInterface("dbkv-json", QLatin1String(DBKVPluginInterface_iid));

    m_dbkv = qobject_cast<DBKVPluginInterface*>(
        Plugins::I()->getPlugin(QLatin1String(DBKVPluginInterface_iid), pluginName));

    if( !m_dbkv )
        LOG_W() << "Failed to load DBKV plugin:" << pluginName;
    else
        LOG_D() << "DBKV plugin set to:" << pluginName;
}

void Core::setDBKVProfilePlugin(const QString& pluginName)
{
    Settings::I()->setting("plugins.dbkv.rocksdb.active", true);
    Plugins::I()->settingActivePlugin("plugins.dbkv.rocksdb.active", "dbkv-rocksdb");
    Plugins::I()->activateInterface("dbkv-rocksdb", QLatin1String(DBKVPluginInterface_iid));

    m_dbkvProfile = qobject_cast<DBKVPluginInterface*>(
        Plugins::I()->getPlugin(QLatin1String(DBKVPluginInterface_iid), pluginName));

    if( !m_dbkvProfile )
        LOG_W() << "Failed to load DBKV profile plugin:" << pluginName;
    else
        LOG_D() << "DBKV profile plugin set to:" << pluginName;
}

void Core::setVFSPlugin(const QString& pluginName)
{
    Settings::I()->setting("plugins.vfs.cake.active", true);
    Plugins::I()->settingActivePlugin("plugins.vfs.cake.active", "vfs-cake");
    Plugins::I()->activateInterface("vfs-cake", QLatin1String("io.stateoftheart.asocial.plugin.VFSPluginInterface"));

    m_vfs = qobject_cast<VFSPluginInterface*>(
        Plugins::I()->getPlugin("io.stateoftheart.asocial.plugin.VFSPluginInterface", pluginName));

    if( !m_vfs )
        LOG_W() << "Failed to load VFS plugin:" << pluginName;
    else
        LOG_D() << "VFS plugin set to:" << pluginName;
}

// ---------------------------------------------------------------------------
// Plugin accessors
// ---------------------------------------------------------------------------

DBKVPluginInterface* Core::getDBKV() const
{
    return m_dbkv;
}
DBKVPluginInterface* Core::getDBKVProfile() const
{
    return m_dbkvProfile;
}
VFSPluginInterface* Core::getVFS() const
{
    return m_vfs;
}

// ---------------------------------------------------------------------------
// Container lifecycle
// ---------------------------------------------------------------------------

bool Core::createContainer(const QString& path, quint64 maxSizeBytes)
{
    if( !m_vfs ) {
        LOG_W() << "No VFS plugin available";
        return false;
    }

    VFSContainerPluginInterface* tmp = m_vfs->openContainer(path, QString(), maxSizeBytes);
    if( !tmp ) {
        LOG_W() << "Container creation failed:" << path;
        return false;
    }
    tmp->close();
    delete tmp;

    m_containerPath = path;
    LOG_D() << "Container created:" << path;
    return true;
}

bool Core::isContainerInitialized() const
{
    return !m_containerPath.isEmpty() && QFile::exists(m_containerPath);
}

QString Core::containerPath() const
{
    return m_containerPath;
}

// ---------------------------------------------------------------------------
// Profile lifecycle
// ---------------------------------------------------------------------------

bool Core::openProfile(const QString& password)
{
    if( !m_vfs || !m_dbkvProfile ) {
        LOG_W() << "VFS or DBKV profile plugin not available";
        return false;
    }

    if( !isContainerInitialized() ) {
        LOG_W() << "Container not initialized";
        return false;
    }

    closeProfile();

    m_container = m_vfs->openContainer(m_containerPath, password);
    if( !m_container ) {
        LOG_W() << "Failed to open VFS container";
        return false;
    }

    const QString dbName = dbFileName();
    QStringList files = m_container->listFiles();

    QString targetFile;
    if( files.contains(dbName) ) {
        targetFile = dbName;
    } else {
        for( const QString& f : files ) {
            if( f.endsWith(QLatin1String(".dat")) ) {
                targetFile = f;
                break;
            }
        }
    }

    if( targetFile.isEmpty() ) {
        LOG_W() << "No database file found in VFS container for this password";
        m_container->close();
        delete m_container;
        m_container = nullptr;
        return false;
    }

    m_dbDevice = m_container->openFile(targetFile);
    if( !m_dbDevice ) {
        LOG_W() << "Failed to open database virtual file:" << targetFile;
        m_container->close();
        delete m_container;
        m_container = nullptr;
        return false;
    }

    m_dbkvProfile->closeDatabase();
    if( !m_dbkvProfile->openDatabase(m_dbDevice) ) {
        LOG_W() << "Failed to open DBKV database from VFS";
        delete m_dbDevice;
        m_dbDevice = nullptr;
        m_container->close();
        delete m_container;
        m_container = nullptr;
        return false;
    }

    asocial::v1::Profile prof;
    if( !m_dbkvProfile->retrieveObject(KV::PROFILE, prof) ) {
        LOG_W() << "No profile record found in KV store";
        m_dbkvProfile->closeDatabase();
        delete m_dbDevice;
        m_dbDevice = nullptr;
        m_container->close();
        delete m_container;
        m_container = nullptr;
        return false;
    }

    m_currentProfileId = prof.uid();

    QStringList personaKeys = m_dbkvProfile->listObjects(KV::PERSONA_PFX);
    for( const QString& key : personaKeys ) {
        asocial::v1::Persona persona;
        if( m_dbkvProfile->retrieveObject(key, persona) ) {
            if( persona.isDefault() ) {
                m_activePersonaId = persona.uid();
                break;
            }
            if( m_activePersonaId.isEmpty() )
                m_activePersonaId = persona.uid();
        }
    }

    LOG_D() << "Profile opened:" << m_currentProfileId << "persona:" << m_activePersonaId;
    emit profileChanged();
    return true;
}

bool Core::createProfile(const QString& password, const QString& displayName)
{
    if( !m_vfs || !m_dbkvProfile ) {
        LOG_W() << "VFS or DBKV profile plugin not available";
        return false;
    }

    if( !isContainerInitialized() ) {
        LOG_W() << "Container not initialized";
        return false;
    }

    closeProfile();

    m_container = m_vfs->openContainer(m_containerPath, password);
    if( !m_container ) {
        LOG_W() << "Failed to open VFS container for new profile";
        return false;
    }

    const QString dbName = dbFileName();
    m_dbDevice = m_container->openFile(dbName, QIODevice::ReadWrite);
    if( !m_dbDevice ) {
        LOG_W() << "Failed to create database virtual file:" << dbName;
        m_container->close();
        delete m_container;
        m_container = nullptr;
        return false;
    }

    m_dbkvProfile->closeDatabase();
    if( !m_dbkvProfile->openDatabase(m_dbDevice) ) {
        LOG_W() << "Failed to open DBKV database on new VFS file";
        delete m_dbDevice;
        m_dbDevice = nullptr;
        m_container->close();
        delete m_container;
        m_container = nullptr;
        return false;
    }

    const auto now = toTimestamp(QDateTime::currentDateTimeUtc());
    const QString profileId = newId();

    asocial::v1::Profile prof;
    prof.setUid(profileId);
    prof.setDisplayName(displayName);
    prof.setCreatedAt(now);
    prof.setUpdatedAt(now);

    if( !m_dbkvProfile->storeObject(KV::PROFILE, prof) ) {
        LOG_W() << "Failed to store profile record";
        m_dbkvProfile->closeDatabase();
        delete m_dbDevice;
        m_dbDevice = nullptr;
        m_container->close();
        delete m_container;
        m_container = nullptr;
        return false;
    }

    const QString personaId = newId();
    asocial::v1::Persona defaultPersona;
    defaultPersona.setUid(personaId);
    defaultPersona.setProfileUid(profileId);
    defaultPersona.setDisplayName(displayName);
    defaultPersona.setIsDefault(true);
    defaultPersona.setCreatedAt(now);
    defaultPersona.setUpdatedAt(now);

    m_dbkvProfile->storeObject(KV::personaKey(personaId), defaultPersona);

    flushProfile();

    m_currentProfileId = profileId;
    m_activePersonaId = personaId;

    LOG_D() << "Profile created:" << profileId << "name:" << displayName;
    emit profileChanged();
    return true;
}

void Core::closeProfile()
{
    if( m_dbkvProfile && m_dbkvProfile->isDatabaseOpen() && m_dbDevice ) {
        m_dbkvProfile->flushDatabase();
        m_dbkvProfile->closeDatabase();
    }

    if( m_dbDevice ) {
        m_dbDevice->close();
        delete m_dbDevice;
        m_dbDevice = nullptr;
    }

    if( m_container ) {
        m_container->close();
        delete m_container;
        m_container = nullptr;
    }

    const bool hadProfile = !m_currentProfileId.isEmpty();
    m_currentProfileId.clear();
    m_activePersonaId.clear();

    if( hadProfile ) {
        LOG_D() << "Profile closed";
        emit profileChanged();
    }
}

bool Core::isProfileOpen() const
{
    return m_container && m_container->isOpen() && m_dbkvProfile && m_dbkvProfile->isDatabaseOpen() && m_dbDevice;
}

bool Core::deleteCurrentProfile(std::function<void(int)> progress)
{
    if( !isProfileOpen() ) {
        LOG_W() << "No profile is open";
        return false;
    }

    QStringList files = m_container->listFiles();
    const int total = files.size();

    if( m_dbkvProfile->isDatabaseOpen() )
        m_dbkvProfile->closeDatabase();

    if( m_dbDevice ) {
        m_dbDevice->close();
        delete m_dbDevice;
        m_dbDevice = nullptr;
    }

    int done = 0;
    for( const QString& f : files ) {
        m_container->deleteFile(f);
        ++done;
        if( progress )
            progress(total > 0 ? (done * 100 / total) : 100);
    }

    m_container->close();
    delete m_container;
    m_container = nullptr;

    m_currentProfileId.clear();
    m_activePersonaId.clear();

    LOG_D() << "Profile deleted";
    emit profileChanged();
    return true;
}

// ---------------------------------------------------------------------------
// Profile data
// ---------------------------------------------------------------------------

asocial::v1::Profile Core::getProfile() const
{
    if( !isProfileOpen() )
        return {};

    asocial::v1::Profile prof;
    if( !m_dbkvProfile->retrieveObject(KV::PROFILE, prof) )
        return {};

    return prof;
}

bool Core::storeProfile(const asocial::v1::Profile& profile)
{
    if( !isProfileOpen() )
        return false;

    auto prof = profile;
    prof.setUpdatedAt(toTimestamp(QDateTime::currentDateTimeUtc()));

    bool ok = m_dbkvProfile->storeObject(KV::PROFILE, prof);
    if( ok )
        flushProfile();
    return ok;
}

QString Core::getCurrentProfileId() const
{
    return m_currentProfileId;
}

QString Core::currentPersonaName() const
{
    if( !isProfileOpen() || m_activePersonaId.isEmpty() )
        return {};

    asocial::v1::Persona persona;
    if( m_dbkvProfile->retrieveObject(KV::personaKey(m_activePersonaId), persona) )
        return persona.displayName();
    return {};
}

QList<asocial::v1::ProfileParameter> Core::listParams() const
{
    QList<asocial::v1::ProfileParameter> result;
    if( !isProfileOpen() )
        return result;

    QStringList keys = m_dbkvProfile->listObjects(KV::PARAM_PFX);
    for( const QString& key : keys ) {
        asocial::v1::ProfileParameter pp;
        if( !m_dbkvProfile->retrieveObject(key, pp) )
            continue;
        result.append(pp);
    }

    std::sort(
        result.begin(),
        result.end(),
        [](const asocial::v1::ProfileParameter& a, const asocial::v1::ProfileParameter& b) {
            return a.key() < b.key();
        });

    return result;
}

asocial::v1::ProfileParameter Core::createParam(const QString& paramKey)
{
    if( !isProfileOpen() )
        return {};

    asocial::v1::ProfileParameter pp;
    pp.setKey(paramKey);

    return pp;
}

asocial::v1::ProfileParameter Core::getParam(const QString& paramKey) const
{
    if( !isProfileOpen() )
        return {};

    asocial::v1::ProfileParameter pp;
    if( !m_dbkvProfile->retrieveObject(KV::paramKey(paramKey), pp) )
        return {};

    return pp;
}

bool Core::storeParam(const asocial::v1::ProfileParameter& param)
{
    if( !isProfileOpen() || param.key().isEmpty() )
        return false;

    auto pp = param;
    bool ok = m_dbkvProfile->storeObject(KV::personaKey(pp.key()), pp);
    if( ok )
        flushProfile();
    return ok;
}

bool Core::deleteParam(const QString& paramKey)
{
    if( !isProfileOpen() )
        return false;

    bool ok = m_dbkvProfile->deleteObject(KV::paramKey(paramKey));
    if( ok )
        flushProfile();
    return ok;
}

// ---------------------------------------------------------------------------
// Export / Import
// ---------------------------------------------------------------------------

QByteArray Core::exportProfile(const QString& /*encryptionPassword*/)
{
    if( !isProfileOpen() )
        return {};

    auto profile = getProfile();
    auto profileParams = listParams();

    auto serializer = new QProtobufJsonSerializer();
    QByteArray result = profile.serialize(serializer);
    for( auto param : profileParams ) {
        result.append('\n');
        result.append(param.serialize(serializer));
    }

    // TODO: Include profile parameters and everything needed to copy profile minimums to another device
    // The idea is to init another devices with the same profile to sync them

    return result;
}

bool Core::importProfile(const QByteArray& data, const QString& /*decryptionPassword*/, const QString& vfsPassword)
{
    QByteArray json = QByteArray::fromBase64(data);
    QJsonDocument doc = QJsonDocument::fromJson(json);
    secureWipe(json);

    if( !doc.isObject() ) {
        LOG_W() << "Invalid import data";
        return false;
    }

    QVariantMap root = doc.object().toVariantMap();
    QVariantMap profMap = root["profile"].toMap();
    QString displayName = profMap.value("display_name", "Imported").toString();

    if( !createProfile(vfsPassword, displayName) ) {
        LOG_W() << "Failed to create profile for import";
        return false;
    }

    // Update profile fields
    if( profMap.contains("bio") ) {
        auto prof = getProfile();
        prof.setBio(profMap["bio"].toString());
        storeProfile(prof);
    }

    flushProfile();
    LOG_D() << "Profile imported successfully";
    return true;
}

// ---------------------------------------------------------------------------
// Settings (system-wide, QSettings-backed)
// ---------------------------------------------------------------------------

// Plugins can only read app settings
QVariant Core::getSetting(const QString& key) const
{
    return Settings::I()->setting(key);
}

// Commented to prevent plugins from manipulating with app settings
bool Core::setSetting(const QString& key, const QVariant& value)
{
    Settings::I()->setting(key, value);
    return true;
}

QStringList Core::listSettings() const
{
    return Settings::I()->listAllKeys();
}

// ---------------------------------------------------------------------------
// Persona
// ---------------------------------------------------------------------------

bool Core::setActivePersona(const QString& personaId)
{
    if( !isProfileOpen() )
        return false;

    asocial::v1::Persona persona;
    if( m_dbkvProfile->retrieveObject(KV::personaKey(personaId), persona) ) {
        m_activePersonaId = personaId;
        LOG_D() << "Active persona set to:" << persona.displayName();
        return true;
    }
    return false;
}

QString Core::activePersonaId() const
{
    return m_activePersonaId;
}

// ---------------------------------------------------------------------------
// Persona CRUD
// ---------------------------------------------------------------------------

QList<asocial::v1::Persona> Core::listPersonas() const
{
    QList<asocial::v1::Persona> result;
    if( !isProfileOpen() )
        return result;

    QStringList keys = m_dbkvProfile->listObjects(KV::PERSONA_PFX);
    for( const QString& key : keys ) {
        asocial::v1::Persona p;
        if( !m_dbkvProfile->retrieveObject(key, p) )
            continue;
        if( p.profileUid() != m_currentProfileId )
            continue;
        result.append(p);
    }

    std::sort(result.begin(), result.end(), [](const asocial::v1::Persona& a, const asocial::v1::Persona& b) {
        if( a.isDefault() != b.isDefault() )
            return a.isDefault();
        return a.displayName() < b.displayName();
    });

    return result;
}

asocial::v1::Persona Core::createPersona(const QString& displayName)
{
    if( !isProfileOpen() )
        return {};

    const auto now = toTimestamp(QDateTime::currentDateTimeUtc());
    const QString uid = newId();

    asocial::v1::Persona p;
    p.setUid(uid);
    p.setProfileUid(m_currentProfileId);
    p.setDisplayName(displayName);
    p.setIsDefault(false);
    p.setCreatedAt(now);
    p.setUpdatedAt(now);

    return p;
}

asocial::v1::Persona Core::getPersona(const QString& personaId) const
{
    if( !isProfileOpen() )
        return {};

    asocial::v1::Persona p;
    if( !m_dbkvProfile->retrieveObject(KV::personaKey(personaId), p) )
        return {};

    return p;
}

bool Core::storePersona(const asocial::v1::Persona& persona)
{
    if( !isProfileOpen() || persona.uid().isEmpty() )
        return false;

    auto p = persona;
    p.setUpdatedAt(toTimestamp(QDateTime::currentDateTimeUtc()));

    bool ok = m_dbkvProfile->storeObject(KV::personaKey(p.uid()), p);
    if( ok )
        flushProfile();
    return ok;
}

bool Core::deletePersona(const QString& personaId)
{
    if( !isProfileOpen() )
        return false;

    // Cascading delete of all dependent entities
    QStringList contactKeys = m_dbkvProfile->listObjects(KV::contactPrefix(personaId));
    for( const QString& key : contactKeys )
        m_dbkvProfile->deleteObject(key);

    QStringList groupKeys = m_dbkvProfile->listObjects(KV::groupPrefix(personaId));
    for( const QString& key : groupKeys ) {
        asocial::v1::Group g;
        if( m_dbkvProfile->retrieveObject(key, g) ) {
            QStringList memberKeys = m_dbkvProfile->listObjects(KV::groupMemberPrefix(g.uid()));
            for( const QString& mk : memberKeys )
                m_dbkvProfile->deleteObject(mk);
        }
        m_dbkvProfile->deleteObject(key);
    }

    QStringList msgKeys = m_dbkvProfile->listObjects(KV::messagePrefix(personaId));
    for( const QString& key : msgKeys )
        m_dbkvProfile->deleteObject(key);

    QStringList evtKeys = m_dbkvProfile->listObjects(KV::eventPrefix(personaId));
    for( const QString& key : evtKeys )
        m_dbkvProfile->deleteObject(key);

    bool ok = m_dbkvProfile->deleteObject(KV::personaKey(personaId));
    if( ok )
        flushProfile();
    return ok;
}

// ---------------------------------------------------------------------------
// Contact CRUD
// ---------------------------------------------------------------------------

QList<asocial::v1::Contact> Core::listContacts() const
{
    QList<asocial::v1::Contact> result;
    if( !isProfileOpen() || m_activePersonaId.isEmpty() )
        return result;

    QStringList keys = m_dbkvProfile->listObjects(KV::contactPrefix(m_activePersonaId));
    for( const QString& key : keys ) {
        asocial::v1::Contact c;
        if( !m_dbkvProfile->retrieveObject(key, c) )
            continue;
        result.append(c);
    }

    std::sort(result.begin(), result.end(), [](const asocial::v1::Contact& a, const asocial::v1::Contact& b) {
        return a.displayName() < b.displayName();
    });
    return result;
}

asocial::v1::Contact Core::createContact(const QString& displayName)
{
    if( !isProfileOpen() || m_activePersonaId.isEmpty() )
        return {};

    const auto now = toTimestamp(QDateTime::currentDateTimeUtc());
    const QString uid = newId();

    asocial::v1::Contact c;
    c.setUid(uid);
    c.setPersonaUid(m_activePersonaId);
    c.setDisplayName(displayName);
    c.setCreatedAt(now);
    c.setUpdatedAt(now);

    return c;
}

asocial::v1::Contact Core::getContact(const QString& contactId) const
{
    if( !isProfileOpen() || m_activePersonaId.isEmpty() )
        return {};

    asocial::v1::Contact c;
    if( !m_dbkvProfile->retrieveObject(KV::contactKey(m_activePersonaId, contactId), c) )
        return {};

    return c;
}

bool Core::storeContact(const asocial::v1::Contact& contact)
{
    if( !isProfileOpen() || contact.uid().isEmpty() || contact.personaUid().isEmpty() )
        return false;

    auto c = contact;
    c.setUpdatedAt(toTimestamp(QDateTime::currentDateTimeUtc()));

    bool ok = m_dbkvProfile->storeObject(KV::contactKey(c.personaUid(), c.uid()), c);
    if( ok )
        flushProfile();
    return ok;
}

bool Core::deleteContact(const QString& contactId)
{
    if( !isProfileOpen() || m_activePersonaId.isEmpty() )
        return false;

    bool ok = m_dbkvProfile->deleteObject(KV::contactKey(m_activePersonaId, contactId));
    if( ok )
        flushProfile();
    return ok;
}

QList<asocial::v1::Contact> Core::searchContacts(const QString& query) const
{
    QList<asocial::v1::Contact> result;
    if( !isProfileOpen() || m_activePersonaId.isEmpty() )
        return result;

    const QString lowerQuery = query.toLower();
    QStringList keys = m_dbkvProfile->listObjects(KV::contactPrefix(m_activePersonaId));
    for( const QString& key : keys ) {
        asocial::v1::Contact c;
        if( !m_dbkvProfile->retrieveObject(key, c) )
            continue;
        if( c.displayName().toLower().contains(lowerQuery) || c.notes().toLower().contains(lowerQuery) )
            result.append(c);
    }
    return result;
}

// ---------------------------------------------------------------------------
// Group CRUD
// ---------------------------------------------------------------------------

QList<asocial::v1::Group> Core::listGroups() const
{
    QList<asocial::v1::Group> result;
    if( !isProfileOpen() || m_activePersonaId.isEmpty() )
        return result;

    QStringList keys = m_dbkvProfile->listObjects(KV::groupPrefix(m_activePersonaId));
    for( const QString& key : keys ) {
        asocial::v1::Group g;
        if( !m_dbkvProfile->retrieveObject(key, g) )
            continue;
        result.append(g);
    }

    std::sort(result.begin(), result.end(), [](const asocial::v1::Group& a, const asocial::v1::Group& b) {
        return a.name() < b.name();
    });
    return result;
}

asocial::v1::Group Core::createGroup(const QString& name)
{
    if( !isProfileOpen() || m_activePersonaId.isEmpty() )
        return {};

    const auto now = toTimestamp(QDateTime::currentDateTimeUtc());
    const QString uid = newId();

    asocial::v1::Group g;
    g.setUid(uid);
    g.setPersonaUid(m_activePersonaId);
    g.setName(name);
    g.setCreatedAt(now);
    g.setUpdatedAt(now);

    return g;
}

asocial::v1::Group Core::getGroup(const QString& groupId) const
{
    if( !isProfileOpen() || m_activePersonaId.isEmpty() )
        return {};

    asocial::v1::Group g;
    if( !m_dbkvProfile->retrieveObject(KV::groupKey(m_activePersonaId, groupId), g) )
        return {};

    return g;
}

bool Core::storeGroup(const asocial::v1::Group& group)
{
    if( !isProfileOpen() || group.uid().isEmpty() || group.personaUid().isEmpty() )
        return false;

    auto g = group;
    g.setUpdatedAt(toTimestamp(QDateTime::currentDateTimeUtc()));

    bool ok = m_dbkvProfile->storeObject(KV::groupKey(g.personaUid(), g.uid()), g);
    if( ok )
        flushProfile();
    return ok;
}

bool Core::deleteGroup(const QString& groupId)
{
    if( !isProfileOpen() || m_activePersonaId.isEmpty() )
        return false;

    QStringList memberKeys = m_dbkvProfile->listObjects(KV::groupMemberPrefix(groupId));
    for( const QString& mk : memberKeys )
        m_dbkvProfile->deleteObject(mk);

    bool ok = m_dbkvProfile->deleteObject(KV::groupKey(m_activePersonaId, groupId));
    if( ok )
        flushProfile();
    return ok;
}

bool Core::addGroupMember(const QString& groupId, const QString& contactId)
{
    if( !isProfileOpen() )
        return false;

    const auto now = toTimestamp(QDateTime::currentDateTimeUtc());
    asocial::v1::GroupMember gm;
    gm.setGroupUid(groupId);
    gm.setContactUid(contactId);
    gm.setRole(QStringLiteral("member"));
    gm.setJoinedAt(now);

    bool ok = m_dbkvProfile->storeObject(KV::groupMemberKey(groupId, contactId), gm);
    if( ok )
        flushProfile();
    return ok;
}

bool Core::removeGroupMember(const QString& groupId, const QString& contactId)
{
    if( !isProfileOpen() )
        return false;

    bool ok = m_dbkvProfile->deleteObject(KV::groupMemberKey(groupId, contactId));
    if( ok )
        flushProfile();
    return ok;
}

QList<asocial::v1::GroupMember> Core::listGroupMembers(const QString& groupId) const
{
    QList<asocial::v1::GroupMember> result;
    if( !isProfileOpen() )
        return result;

    QStringList keys = m_dbkvProfile->listObjects(KV::groupMemberPrefix(groupId));
    for( const QString& key : keys ) {
        asocial::v1::GroupMember gm;
        if( !m_dbkvProfile->retrieveObject(key, gm) )
            continue;
        result.append(gm);
    }
    return result;
}

// ---------------------------------------------------------------------------
// Message CRUD
// ---------------------------------------------------------------------------

QList<asocial::v1::Message> Core::listMessages(int limit) const
{
    QList<asocial::v1::Message> result;
    if( !isProfileOpen() || m_activePersonaId.isEmpty() )
        return result;

    // Keys are time-ordered thanks to the zero-padded millisecond prefix
    QStringList keys = m_dbkvProfile->listObjects(KV::messagePrefix(m_activePersonaId));

    // Iterate in reverse for newest-first
    for( int i = keys.size() - 1; i >= 0 && result.size() < limit; --i ) {
        asocial::v1::Message m;
        if( !m_dbkvProfile->retrieveObject(keys[i], m) )
            continue;
        result.append(m);
    }
    return result;
}

asocial::v1::Message Core::createMessage(const QString& recipientId, const QString& body, const QString& recipientType)
{
    if( !isProfileOpen() || m_activePersonaId.isEmpty() )
        return {};

    const QDateTime now = QDateTime::currentDateTimeUtc();
    const auto ts = toTimestamp(now);
    const QString uid = newId();

    asocial::v1::Message msg;
    msg.setUid(uid);
    msg.setPersonaUid(m_activePersonaId);
    msg.setThreadUid(newId());
    msg.setRecipientType(recipientType);
    msg.setRecipientUid(recipientId);
    msg.setBody(body);
    msg.setCreatedAt(ts);

    return msg;
}

asocial::v1::Message Core::getMessage(const QString& messageId) const
{
    if( !isProfileOpen() || m_activePersonaId.isEmpty() )
        return {};

    QStringList keys = m_dbkvProfile->listObjects(KV::messagePrefix(m_activePersonaId));
    for( const QString& key : keys ) {
        asocial::v1::Message m;
        if( !m_dbkvProfile->retrieveObject(key, m) )
            continue;
        if( m.uid() == messageId )
            return m;
    }
    return {};
}

bool Core::storeMessage(const asocial::v1::Message& message)
{
    if( !isProfileOpen() || message.uid().isEmpty() || message.personaUid().isEmpty() )
        return false;

    const qint64 msEpoch = timestampToMsEpoch(message.createdAt());
    const QString key = KV::messageKey(message.personaUid(), msEpoch, message.uid());

    bool ok = m_dbkvProfile->storeObject(key, message);
    if( ok )
        flushProfile();
    return ok;
}

bool Core::deleteMessage(const QString& messageId)
{
    if( !isProfileOpen() || m_activePersonaId.isEmpty() )
        return false;

    QStringList keys = m_dbkvProfile->listObjects(KV::messagePrefix(m_activePersonaId));
    for( const QString& key : keys ) {
        if( key.endsWith('/' + messageId) ) {
            bool ok = m_dbkvProfile->deleteObject(key);
            if( ok )
                flushProfile();
            return ok;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Event CRUD
// ---------------------------------------------------------------------------

QList<asocial::v1::Event> Core::listEvents() const
{
    QList<asocial::v1::Event> result;
    if( !isProfileOpen() || m_activePersonaId.isEmpty() )
        return result;

    QStringList keys = m_dbkvProfile->listObjects(KV::eventPrefix(m_activePersonaId));
    for( const QString& key : keys ) {
        asocial::v1::Event e;
        if( !m_dbkvProfile->retrieveObject(key, e) )
            continue;
        result.append(e);
    }

    std::sort(result.begin(), result.end(), [](const asocial::v1::Event& a, const asocial::v1::Event& b) {
        QString dateA = a.hasStarted() ? timestampToIso(a.started()) : QString();
        QString dateB = b.hasStarted() ? timestampToIso(b.started()) : QString();
        return dateA > dateB;
    });
    return result;
}

asocial::v1::Event Core::createEvent(const QString& title, const QString& date)
{
    if( !isProfileOpen() || m_activePersonaId.isEmpty() )
        return {};

    const auto now = toTimestamp(QDateTime::currentDateTimeUtc());
    const QString uid = newId();

    asocial::v1::Event e;
    e.setUid(uid);
    e.setPersonaUid(m_activePersonaId);
    e.setTitle(title);
    e.setCreatedAt(now);
    e.setUpdatedAt(now);

    QDateTime dt = QDateTime::fromString(date, Qt::ISODate);
    if( !dt.isValid() )
        dt = QDateTime::fromString(date, QStringLiteral("yyyy-MM-dd"));
    if( dt.isValid() )
        e.setStarted(toTimestamp(dt));

    return e;
}

asocial::v1::Event Core::getEvent(const QString& eventId) const
{
    if( !isProfileOpen() || m_activePersonaId.isEmpty() )
        return {};

    asocial::v1::Event e;
    if( !m_dbkvProfile->retrieveObject(KV::eventKey(m_activePersonaId, eventId), e) )
        return {};

    return e;
}

bool Core::storeEvent(const asocial::v1::Event& event)
{
    if( !isProfileOpen() || event.uid().isEmpty() || event.personaUid().isEmpty() )
        return false;

    auto e = event;
    e.setUpdatedAt(toTimestamp(QDateTime::currentDateTimeUtc()));

    bool ok = m_dbkvProfile->storeObject(KV::eventKey(e.personaUid(), e.uid()), e);
    if( ok )
        flushProfile();
    return ok;
}

bool Core::deleteEvent(const QString& eventId)
{
    if( !isProfileOpen() || m_activePersonaId.isEmpty() )
        return false;

    bool ok = m_dbkvProfile->deleteObject(KV::eventKey(m_activePersonaId, eventId));
    if( ok )
        flushProfile();
    return ok;
}

// ---------------------------------------------------------------------------
// App
// ---------------------------------------------------------------------------

void Core::setApp(QCoreApplication* app)
{
    m_app = app;
}

void Core::exit()
{
    closeProfile();
    m_app->exit(0);
}

void Core::init()
{
    // Open container if it already exists
    QString container_path = Settings::I()->setting("vfs.container.path").toString();
    if( QFile::exists(container_path) ) {
        createContainer(container_path, 0);
    }
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

QString Core::dbFileName() const
{
    if( m_dbkvProfile ) {
        auto* pi = dynamic_cast<PluginInterface*>(m_dbkvProfile);
        if( pi )
            return pi->name() + QStringLiteral(".dat");
    }
    return QStringLiteral("dbkv-rocksdb.dat");
}
