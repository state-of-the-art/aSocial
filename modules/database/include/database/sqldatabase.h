#ifndef SQLDATABASE_H
#define SQLDATABASE_H

#include <QObject>
#include <QtSql/QSqlDatabase>

class SqlDatabase : public QObject
{
    Q_OBJECT

  public:
    explicit SqlDatabase(QObject *parent = 0, QString name = "", QString path = "");
    ~SqlDatabase();

    void open(const QString &password = "");
    bool table(const QString &name, const QStringList &fields);

    bool backup();   // Database backup
    bool restore();  // DB backup restore (in case of upgrade fails)

    QString dumpSchema();  // Database dump schema
    QString dumpData();    // Database dump data

  protected:
    QString m_name;
    QString m_path;

    QSqlDatabase m_db;
};

#endif  // SQLDATABASE_H
