#ifndef FEDATABASE_H
#define FEDATABASE_H

#include <QJsonArray>
#include <QJsonObject>

#include "database/sqldatabase.h"

class PrivKey;
class AccountDatabase;

class FEDatabase : public SqlDatabase
{
    Q_OBJECT

  public:
    FEDatabase(QObject *parent, QString name, QString path);
    ~FEDatabase();

    void open(const QString &password);

    QJsonArray getAccounts();
    int createAccount(const QJsonObject &account, const QString &password);
    bool openAccount(const int id);
    void closeAccount();

    AccountDatabase *getCurrentAccount();

  private:
    qint16 version();
    void setVersion(const qint16 version, const QString &description);
    void upgrade(const qint16 from_version);

    PrivKey *getAccountPassKey(const int id);

    qint16 m_version;  // Current database version

    AccountDatabase *m_active_account;
};

#endif  // FEDATABASE_H
