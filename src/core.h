#ifndef CORE_H
#define CORE_H

#include <QObject>
#include <QVariantMap>
#include <QUuid>
#include "CoreInterface.h"
#include "plugin/DatabasePluginInterface.h"

class Core : public QObject, public CoreInterface
{
    Q_OBJECT
    Q_INTERFACES(CoreInterface)

public:
    inline static Core* I() {
        if (s_pInstance == nullptr)
            s_pInstance = new Core();
        return s_pInstance;
    }
    inline static void destroyI() { delete s_pInstance; }

    // Database management
    DatabasePluginInterface* getDatabase() const override;
    void setDatabasePlugin(const QString& pluginName);

    // Profile management
    QString getCurrentProfileId() const override;
    void setCurrentProfileId(const QString& profileId) override;

    // Profile operations
    QString createProfile(const QString& name) override;
    QString importProfile(const QString& serializedData) override;
    QString exportProfile(const QString& profileId) override;
    bool deleteProfile(const QString& profileId) override;
    QStringList listProfiles() override;
    QVariantMap getProfileInfo(const QString& profileId) override;

signals:
    void currentProfileChanged(const QString& profileId) override;

private:
    explicit Core(QObject *parent = nullptr);
    ~Core() override;

    static Core* s_pInstance;

    DatabasePluginInterface* m_database;
    QString m_currentProfileId;
};

#endif // CORE_H
