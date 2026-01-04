#ifndef COREINTERFACE_H
#define COREINTERFACE_H

#include <QObject>
#include <QVariantMap>
#include <QStringList>

class DatabasePluginInterface;

/**
 * Abstract interface for Core functionality that plugins can use
 * This ensures binary compatibility when Core is extended
 */
class CoreInterface
{
public:
    virtual ~CoreInterface() {}

    // Database management
    virtual DatabasePluginInterface* getDatabase() const = 0;

    // Profile management
    virtual QString getCurrentProfileId() const = 0;
    virtual void setCurrentProfileId(const QString& profileId) = 0;

    // Profile operations
    virtual QString createProfile(const QString& name) = 0;
    virtual QString importProfile(const QString& serializedData) = 0;
    virtual QString exportProfile(const QString& profileId) = 0;
    virtual bool deleteProfile(const QString& profileId) = 0;
    virtual QStringList listProfiles() = 0;
    virtual QVariantMap getProfileInfo(const QString& profileId) = 0;

signals:
    virtual void currentProfileChanged(const QString& profileId) = 0;
};

Q_DECLARE_INTERFACE(CoreInterface, "io.stateoftheart.asocial.CoreInterface")

#endif // COREINTERFACE_H
