#ifndef PLUGIN_H
#define PLUGIN_H

#include <QObject>
#include "plugin/DBSQLPluginInterface.h"

class Plugin : public QObject, public DBSQLPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "io.stateoftheart.asocial.plugin.DBSQLSqlCipherPlugin")
    Q_INTERFACES(DBSQLPluginInterface PluginInterface)

public:
    Plugin() {}
    static Plugin *s_pInstance;
    ~Plugin() override {}

    // PluginInterface
    Q_INVOKABLE QString name() const override;
    QStringList requirements() const override;
    bool init() override; // Warning: executing multiple times for each interface
    bool deinit() override;
    bool configure() override;

signals:
    void appNotice(QString msg) override;
    void appWarning(QString msg) override;
    void appError(QString msg) override;
};
#endif // PLUGIN_H
