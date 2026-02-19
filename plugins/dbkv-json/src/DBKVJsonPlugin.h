#ifndef DATABASEJSONPLUGIN_H
#define DATABASEJSONPLUGIN_H

#include <QObject>
#include <QDir>

#include "plugin/DBKVPluginInterface.h"

class DBKVJsonPlugin : public QObject, public DBKVPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "io.stateoftheart.asocial.plugin.DBKVJsonPlugin")
    Q_INTERFACES(DBKVPluginInterface PluginInterface)

public:
    DBKVJsonPlugin();
    ~DBKVJsonPlugin() override;

    // PluginInterface
    Q_INVOKABLE QString name() const override;
    QStringList requirements() const override;
    bool init() override;
    bool deinit() override;
    bool configure() override;

    // DBKVPluginInterface
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
