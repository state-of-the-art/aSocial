#ifndef DATABASEJSONPLUGIN_H
#define DATABASEJSONPLUGIN_H

#include <QObject>
#include <QDir>

#include "plugin/DatabasePluginInterface.h"

class DatabaseJsonPlugin : public QObject, public DatabasePluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "io.stateoftheart.asocial.plugin.DatabaseJsonPlugin")
    Q_INTERFACES(DatabasePluginInterface PluginInterface)

public:
    DatabaseJsonPlugin();
    ~DatabaseJsonPlugin() override;

    // PluginInterface
    Q_INVOKABLE QString name() const override;
    QStringList requirements() const override;
    bool init() override;
    bool deinit() override;
    bool configure() override;

    // DatabasePluginInterface
    QStringList listObjects(const QString& prefix) override;
    bool storeObject(const QString& key, QVariantMap& object) override;
    bool retrieveObject(const QString& key, QVariantMap& object) override;
    bool objectExists(const QString& key) override;
    bool deleteObject(const QString& key) override;

signals:
    void appNotice(QString msg) override;
    void appWarning(QString msg) override;
    void appError(QString msg) override;

private:
    QDir m_dataDir;
    QString getObjectFilePath(const QString& key) const;
};

#endif // DATABASEJSONPLUGIN_H
