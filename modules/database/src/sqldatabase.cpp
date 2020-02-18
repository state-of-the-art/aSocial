#include "database/sqldatabase.h"

#include <QDebug>
#include <QDir>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>

#include "qsql_sqlcipher_p.h"

SqlDatabase::SqlDatabase(QObject *parent, QString name, QString path)
    : QObject(parent)
    , m_name(name)
    , m_path(path)
{
    qDebug() << "Create SqlDatabase" << m_name;
}

SqlDatabase::~SqlDatabase()
{
    m_db.close();
    qDebug() << "Destroy SqlDatabase" << m_name;
}

void SqlDatabase::open(const QString &password)
{
    if( m_db.isOpen() )
        return;

    if( m_path.isEmpty() || m_name.isEmpty() ) {
        qCritical() << "Unable to open DB with empty path or file:" << m_path << m_name;
        return;
    }

    QDir(m_path).mkpath(".");
    QString db_path = m_path + "/" + m_name;
    qDebug() << "Open SQL DB:" << db_path;

    m_db = QSqlDatabase::addDatabase(new QSQLCipherDriver(), db_path);
    m_db.setDatabaseName(db_path);
    m_db.setPassword(password);

    if( ! m_db.open() ) {
        qCritical() << m_db.lastError();
        qFatal("Unable to open sql database");
    }

    // TODO: remove DEBUG dump schema & data
    qDebug() << dumpSchema();
    qDebug() << dumpData();
}

bool SqlDatabase::table(const QString &name, const QStringList &fields)
{
    qDebug() << "Table query:" << name;
    QSqlQuery query(m_db);

    bool result = query.exec(QString("CREATE TABLE IF NOT EXISTS %1 (%2)")
                                 .arg(m_db.driver()->escapeIdentifier(name, QSqlDriver::TableName), fields.join(',')));
    if( ! result )
        qCritical() << "DB query failed" << query.lastQuery() << query.lastError().text();

    return result;
}

bool SqlDatabase::backup()
{
    qDebug("Backup database started");

    qDebug() << "Close database" << m_name;
    m_db.close();

    qDebug("Starting SqlDatabase backup");

    QString backup_name = m_path + "/backup_" + m_name;

    // Remove previous backup
    if( QFile::exists(backup_name) )
        QFile::remove(backup_name);

    // Copy current database to backup
    if( ! QFile::copy(m_path + "/" + m_name, backup_name) ) {
        QFile::remove(backup_name);
        qFatal("Unable to create backup of the database");
    }

    qDebug("Database backup done");

    qDebug() << "Reopen database" << m_name;

    if( ! m_db.open() ) {
        qCritical() << m_db.lastError();
        qFatal("Unable to open sql database");
    }

    return true;
}

bool SqlDatabase::restore()
{
    qDebug("Restore database backup started");

    qDebug() << "Close database" << m_name;
    m_db.close();

    qDebug("Starting SqlDatabase backup");

    bool result = true;

    QString backup_name = m_path + "/backup_" + m_name;
    QString failed_name = m_path + "/failed_" + m_name;

    if( QFile::exists(backup_name) ) {
        // Move failed migration
        if( ! QFile::rename(m_path + "/" + m_name, failed_name) )
            qFatal("Unable to rename failed migration file of the database");

        // Move backup back to original place
        if( ! QFile::rename(backup_name, m_path + "/" + m_name) )
            qFatal("Unable to restore backup of the database");

        qDebug("Database restore backup done");
    } else {
        qCritical() << "Can't find backup file to restore" << backup_name;
        result = false;
    }

    qDebug() << "Open database" << m_name;

    if( ! m_db.open() ) {
        qCritical() << m_db.lastError();
        qFatal("Unable to open sql database");
    }

    return result;
}

QString SqlDatabase::dumpSchema()
{
    QString out;
    QSqlQuery query(m_db);

    QStringList tables = m_db.tables();
    for( QString table : tables ) {
        out += QString("Table: %1\n").arg(table);
        if( ! query.exec(QString("PRAGMA table_info(\"%1\")").arg(table)) )
            qCritical() << "DB query failed" << query.lastQuery() << query.lastError().text();
        while( query.next() )
            out += QString("  %1 %2 %3 %4\n")
                       .arg(query.value(0).toInt())
                       .arg(query.value(1).toString())
                       .arg(query.value(2).toInt())
                       .arg(query.value(3).toInt());
    }

    return out;
}

QString SqlDatabase::dumpData()
{
    QString out;
    QSqlQuery query(m_db);

    QStringList tables = m_db.tables();
    for( QString table : tables ) {
        out += QString("Table: %1\n").arg(table);
        query.exec(QString("SELECT rowid, * FROM \"%1\"").arg(table));
        while( query.next() ) {
            int fields = query.record().count();
            for( int i = 0; i < fields; i++ ) {
                QVariant var = query.value(i);
                if( var.isNull() )
                    out += "\tNULL";
                else if( var.type() == QVariant::String )
                    out += QString("\t\"%1\"").arg(var.toString());
                else if( var.type() == QVariant::ByteArray )
                    out += QString("\t0x%1").arg(QString(var.toByteArray().toHex()));
                else
                    out += QString("\t%1").arg(var.toString());
            }
            out += "\n";
        }
    }

    return out;
}
