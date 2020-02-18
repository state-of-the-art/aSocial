#ifndef FRONTEND_H
#define FRONTEND_H

#include <QColor>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>

class QGuiApplication;
class QQmlApplicationEngine;
class QTranslator;
class QQmlContext;

class PrivKey;
class Backend;
class FEDatabase;
class AccountDatabase;
class WDate;

class Frontend : public QObject
{
    Q_OBJECT

  public:
    inline static Frontend *I()
    {
        if( s_pInstance == NULL )
            s_pInstance = new Frontend();
        return s_pInstance;
    }
    inline static void destroyI() { delete s_pInstance; }
    static void signalHandler(int signal);

    void init(QGuiApplication *app);

    void setLocale(QString locale);

    Q_INVOKABLE QString passwordGetWait(const QString &description, const bool create = false);

    Q_INVOKABLE QJsonArray getAccounts();
    Q_INVOKABLE int createAccount(const QJsonObject &account, const QString &password);
    Q_INVOKABLE bool openAccount(const int id);
    Q_INVOKABLE void closeAccount();

    Q_INVOKABLE AccountDatabase *getCurrentAccount();

    Q_INVOKABLE QColor getTextColor(QColor background_color);

  signals:
    void postinitDone();
    void requestPassword(const QString description, const bool create);
    void donePassword(int code);

  public slots:
    void deleteMe() { Frontend::destroyI(); }
    void responsePassword(const QString password);

  private slots:
    void postinit();

  private:
    explicit Frontend(QObject *parent = 0);
    ~Frontend();

    static Frontend *s_pInstance;

    void initInterface();
    void initLocale();
    void initEngine();

    void postinitDevicePassKey();
    void postinitBackend();
    void postinitDatabase();

    QQmlApplicationEngine *m_engine;
    QGuiApplication *m_app;
    QQmlContext *m_context;
    QTranslator *m_translator;

    QString m_password;

    PrivKey *m_device_passkey;
    Backend *m_backend;
    FEDatabase *m_database;

    WDate *m_date_wrapper;
};

#endif  // FRONTEND_H
