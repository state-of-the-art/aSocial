#ifndef PLUGIN_H
#define PLUGIN_H

#include <QObject>

#include "plugin/UiPluginInterface.h"

class UiWorker;

class Plugin : public QObject, public UiPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "io.stateoftheart.handy3dscanner.plugins.UiCmdPlugin")
    Q_INTERFACES(UiPluginInterface PluginInterface)

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

    // UiPluginInterface
    bool startUI() override;
    bool stopUI() override;

signals:
    void appNotice(QString msg) override;
    void appWarning(QString msg) override;
    void appError(QString msg) override;

private:
    UiWorker* ui_thread;
};
#endif // PLUGIN_H
