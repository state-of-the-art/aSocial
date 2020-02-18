#ifndef ACCOUNTDATABASE_H
#define ACCOUNTDATABASE_H

#include <QJsonArray>
#include <QJsonObject>

#include "crypto/privkey.h"
#include "database/sqldatabase.h"

class AccountDatabase : public SqlDatabase
{
    Q_OBJECT

  public:
    AccountDatabase(QObject *parent, QString name, QString path);

    void open(const QString &password);

    Q_INVOKABLE qint64 createProfile(const QJsonObject &profile);
    Q_INVOKABLE QJsonObject getProfile(const qint64 id);
    Q_INVOKABLE bool updateProfileData(const QJsonObject &profile);

    Q_INVOKABLE qint64 createEvent(const QJsonObject &event);
    Q_INVOKABLE QJsonArray getEvents(const QJsonArray ids);
    Q_INVOKABLE QJsonArray findEvents(const qint64 occur_from, const qint64 occur_to, const int type = -1,
                                      const qint64 owner = -1, const qint64 recipient = -1, const qint16 limit = -1);
    Q_INVOKABLE bool updateEvent(const QJsonObject &event);

    Q_INVOKABLE QString getTypeName(const qint16 id);
    Q_INVOKABLE qint16 getTypeId(const QString &name);
    Q_INVOKABLE QJsonArray getTypes(const qint16 parent_id = -1);

    Q_INVOKABLE QString createAddress();

    Q_INVOKABLE qint64 storeKey(const PrivKey *key, const QString description);

  private:
    qint16 version();
    void setVersion(const qint16 version, const QString &description);
    void upgrade(const qint16 from_version);

    qint16 m_version;  // Current database version
};

#endif  // ACCOUNTDATABASE_H
