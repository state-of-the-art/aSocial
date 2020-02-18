#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

class Settings : public QObject
{
    Q_OBJECT

  public:
    inline static Settings *I()
    {
        if( s_pInstance == NULL )
            s_pInstance = new Settings();
        return s_pInstance;
    }
    inline static void destroyI() { delete s_pInstance; }

    inline static void setConfigFile(QString &path) { s_pInstance = new Settings(path); }

    Q_INVOKABLE QVariant setting(QString key, QVariant value = QVariant());
    Q_INVOKABLE bool isNull(QString key);
    void setDefault(QString key, QVariant value);

  signals:
    void settingChanged(QString key);

  private:
    explicit Settings(QObject *parent = 0);
    Settings(QString &path, QObject *parent = 0);
    ~Settings();

    QSettings m_settings;
    static Settings *s_pInstance;
};

#endif  // SETTINGS_H
