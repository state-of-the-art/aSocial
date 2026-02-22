#include "DBKVJsonPlugin.h"

#include <QStandardPaths>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QLoggingCategory>
#include <QByteArray>

Q_LOGGING_CATEGORY(Djp, "DBKVJsonPlugin")

DBKVJsonPlugin::DBKVJsonPlugin()
    : m_dataDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" + name())
{
}

DBKVJsonPlugin::~DBKVJsonPlugin()
{
}

QString DBKVJsonPlugin::name() const
{
    return "dbkv-json";
}

QStringList DBKVJsonPlugin::requirements() const
{
    return QStringList();
}

bool DBKVJsonPlugin::init()
{
    qCDebug(Djp) << "Initializing dbkv-json plugin";

    // Create data directory if it doesn't exist
    if (!m_dataDir.exists()) {
        if (!m_dataDir.mkpath(".")) {
            qCWarning(Djp) << "Failed to create data directory:" << m_dataDir.absolutePath();
            setInitialized(false);
            return false;
        }
    }

    qCDebug(Djp) << "Data directory:" << m_dataDir.absolutePath();
    setInitialized(true);
    return true;
}

bool DBKVJsonPlugin::deinit()
{
    qCDebug(Djp) << "Deinitializing dbkv-json plugin";
    setInitialized(false);
    return true;
}

bool DBKVJsonPlugin::configure()
{
    qCDebug(Djp) << "Configuring dbkv-json plugin";
    return true;
}

QStringList DBKVJsonPlugin::listObjects(const QString &prefix)
{
    QStringList profiles;
    QStringList filters = {"profile_*.json"};
    QFileInfoList files = m_dataDir.entryInfoList(filters, QDir::Files);

    for (const QFileInfo& file : files) {
        QString fileName = file.baseName(); // Remove .json extension
        if (fileName.startsWith("profile_")) {
            QString profileId = fileName.mid(8); // Remove "profile_" prefix
            profiles.append(profileId);
        }
    }

    return profiles;
}

bool DBKVJsonPlugin::storeObject(const QString& key, QVariantMap& object)
{
    if (!isInitialized()) {
        qCWarning(Djp) << "Plugin not initialized";
        return false;
    }

    QString serializedData = QJsonDocument::fromVariant(object).toJson();
    if (serializedData.isEmpty()) {
        qCWarning(Djp) << "Failed to serialize object for key:" << key;
        return false;
    }

    QString filePath = getObjectFilePath(key);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qCWarning(Djp) << "Failed to open file for writing:" << filePath;
        return false;
    }

    file.write(serializedData.toUtf8());
    file.close();

    return true;
}

bool DBKVJsonPlugin::retrieveObject(const QString& key, QVariantMap& object)
{
    if (!isInitialized()) {
        qCWarning(Djp) << "Plugin not initialized";
        return false;
    }

    QString filePath = getObjectFilePath(key);
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(Djp) << "Failed to open file for reading:" << filePath;
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    object = doc.object().toVariantMap();

    return true;
}

bool DBKVJsonPlugin::objectExists(const QString& key)
{
    if (!isInitialized()) {
        return false;
    }

    QString filePath = getObjectFilePath(key);
    return QFile::exists(filePath);
}

bool DBKVJsonPlugin::deleteObject(const QString& key)
{
    if (!isInitialized()) {
        qCWarning(Djp) << "Plugin not initialized";
        return false;
    }

    QString filePath = getObjectFilePath(key);
    if (QFile::exists(filePath)) {
        return QFile::remove(filePath);
    }

    return true; // Object doesn't exist, consider it deleted
}

QString DBKVJsonPlugin::getObjectFilePath(const QString& key) const
{
    return m_dataDir.absoluteFilePath(key + ".json");
}
