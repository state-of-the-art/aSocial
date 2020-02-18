#ifndef BEDATABASE_H
#define BEDATABASE_H

#include "database/nosqldatabase.h"

class PrivKey;

class BEDatabase : public NoSqlDatabase
{
    Q_OBJECT

  public:
    BEDatabase(QObject *parent, const QString &name, const QString &path);

    void open();
    long version();
    void setVersion(const long version);

    PrivKey *getDeviceKey();
    void storeDeviceKey(const PrivKey *key);

    void upgrade(const long from_version);

  private:
    long m_version;  // Current database version
};

#endif  // BEDATABASE_H
