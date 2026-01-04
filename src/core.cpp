#include "core.h"
#include "CoreInterface.h"
#include "plugins.h"
#include "settings.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(Cc, "Core")

Core* Core::s_pInstance = nullptr;

Core::Core(QObject *parent)
    : QObject(parent)
    , m_database(nullptr)
{
    qCDebug(Cc) << "Core singleton created";
}

Core::~Core()
{
    qCDebug(Cc) << "Core singleton destroyed";
}

DatabasePluginInterface* Core::getDatabase() const
{
    return m_database;
}

void Core::setDatabasePlugin(const QString& pluginName)
{
    // Enable database plugin
    Settings::I()->setting("plugins.database.json.active", true);
    Plugins::I()->settingActivePlugin("plugins.database.json.active", "database-json");
    Plugins::I()->activateInterface("database-json", QLatin1String("io.stateoftheart.asocial.plugin.DatabasePluginInterface"));

    m_database = qobject_cast<DatabasePluginInterface*>(
        Plugins::I()->getPlugin("io.stateoftheart.asocial.plugin.DatabasePluginInterface", pluginName));

    if (!m_database) {
        qCWarning(Cc) << "Failed to load database plugin:" << pluginName;
    } else {
        qCDebug(Cc) << "Database plugin set to:" << pluginName;
    }
}

QString Core::getCurrentProfileId() const
{
    return m_currentProfileId;
}

void Core::setCurrentProfileId(const QString& profileId)
{
    if (m_currentProfileId != profileId) {
        m_currentProfileId = profileId;
        emit currentProfileChanged(profileId);
        qCDebug(Cc) << "Current profile changed to:" << profileId;
    }
}

QString Core::createProfile(const QString& name)
{
    if (!m_database) {
        qCWarning(Cc) << "No database plugin available";
        return QString();
    }

    QUuid profileId = QUuid::createUuid();
    QString profileIdStr = profileId.toString(QUuid::WithoutBraces);

    QVariantMap profileData;
    profileData["id"] = profileIdStr;
    profileData["name"] = name;
    profileData["created"] = QDateTime::currentDateTime();

    if (!m_database->storeObject("profile_" + profileIdStr, profileData)) {
        qCWarning(Cc) << "Failed to create profile storage object";
        return QString();
    }

    qCDebug(Cc) << "Created profile:" << profileIdStr << "with name:" << name;

    return profileIdStr;
}

QString Core::importProfile(const QString& serializedData)
{
    if (!m_database) {
        qCWarning(Cc) << "No database plugin available";
        return QString();
    }

    QByteArray jsonData = QByteArray::fromBase64(serializedData.toUtf8());
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);

    if (!doc.isObject()) {
        qCWarning(Cc) << "Invalid serialized profile data";
        return QString();
    }

    QVariantMap profileData = doc.object().toVariantMap();
    QString profileId = profileData["id"].toString();

    if (profileId.isEmpty()) {
        qCWarning(Cc) << "Profile data missing ID";
        return QString();
    }

    // Check if profile already exists
    if (m_database->objectExists("profile_" + profileId)) {
        qCWarning(Cc) << "Profile already exists:" << profileId;
        return QString();
    }

    // Store profile metadata
    if (!m_database->storeObject("profile_" + profileId, profileData)) {
        qCWarning(Cc) << "Unable to store profile:" << profileId;
        return QString();
    }

    qCDebug(Cc) << "Imported profile:" << profileId;

    return profileId;
}

QString Core::exportProfile(const QString& profileId)
{
    if (!m_database) {
        qCWarning(Cc) << "No database plugin available";
        return QString();
    }

    QByteArray jsonData = QJsonDocument::fromVariant(getProfileInfo(profileId)).toJson();

    return QString(jsonData.toBase64());
}

bool Core::deleteProfile(const QString& profileId)
{
    if (!m_database) {
        qCWarning(Cc) << "No database plugin available";
        return false;
    }

    if (m_database->objectExists("profile_" + profileId)) {
        if (!m_database->deleteObject("profile_" + profileId)) {
            qCWarning(Cc) << "Failed to delete profile:" << profileId;
            return false;
        }
    }

    qCDebug(Cc) << "Deleted profile:" << profileId;
    return true;
}

QStringList Core::listProfiles()
{
    if (!m_database) {
        qCWarning(Cc) << "No database plugin available";
        return QStringList();
    }

    return m_database->listObjects("profile_");
}

QVariantMap Core::getProfileInfo(const QString& profileId)
{
    if (!m_database) {
        qCWarning(Cc) << "No database plugin available";
        return QVariantMap();
    }

    QVariantMap object;
    if (!m_database->retrieveObject("profile_" + profileId, object)) {
        qCWarning(Cc) << "Failed to get profile:" << profileId;
        return QVariantMap();
    }

    return object;
}
