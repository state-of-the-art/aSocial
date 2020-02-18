#include "database/nosqldatabase.h"

#include <QDebug>
#include <QDir>

#include "leveldb/db.h"

NoSqlDatabase::NoSqlDatabase(QObject *parent, QString name, QString path)
    : QObject(parent)
    , m_name(name)
    , m_path(path)
    , m_db(NULL)
{
    qDebug() << "Create NoSqlDatabase" << m_name;
}

NoSqlDatabase::~NoSqlDatabase()
{
    delete m_db;
    qDebug() << "Destroy NoSqlDatabase" << m_name;
}

void NoSqlDatabase::open()
{
    if( m_db != NULL ) {
        qWarning() << "Database" << m_name << "is already open";
        return;
    }

    if( m_path.isEmpty() || m_name.isEmpty() ) {
        qCritical() << "Unable to open DB with empty path or file:" << m_path << m_name;
        return;
    }

    QDir(m_path).mkpath(".");
    QString db_path = m_path + "/" + m_name;
    qDebug() << "Open NoSQL DB:" << db_path;

    leveldb::Options options;
    options.create_if_missing = true;
    if( ! leveldb::DB::Open(options, db_path.toUtf8().constData(), &m_db).ok() ) {
        qWarning() << "Unable to open database" << db_path;
        return;
    }
}

void NoSqlDatabase::close()
{
    qDebug() << "Close NoSQL DB:" << m_name;
    delete m_db;
    m_db = NULL;
}

bool NoSqlDatabase::fetch(const QString &key, QByteArray &data)
{
    qDebug() << "Fetching data" << key;
    ::std::string value;
    if( ! m_db->Get(leveldb::ReadOptions(), key.toStdString(), &value).ok() ) {
        qDebug() << "Data not found" << key;
        return false;
    }

    data.setRawData(value.c_str(), value.size());
    data.detach();
    return true;
}

QByteArray NoSqlDatabase::fetchStore(const QString &key, const QByteArray &val)
{
    qDebug() << "Fetching with store data" << key;
    QByteArray data;
    if( ! fetch(key, data) )
        store(key, val);

    return data;
}

void NoSqlDatabase::store(const QString &key, const QByteArray &val)
{
    qDebug() << "Store data" << key;
    std::string value(val.constData(), val.length());
    m_db->Put(leveldb::WriteOptions(), key.toStdString(), value);
}

void NoSqlDatabase::backup()
{
    close();

    qDebug("Starting NoSqlDatabase backup");

    QString backup_from = m_path + "/" + m_name;
    QDir backup_name(m_path + "/backup_" + m_name);

    // Remove previous backup
    if( backup_name.exists() )
        backup_name.removeRecursively();

    backup_name.mkpath(".");
    QStringList files = QDir(backup_from).entryList(QDir::Files);
    for( const QString &file : files ) {
        if( ! QFile::copy(backup_from + "/" + file, backup_name.path() + "/" + file) ) {
            backup_name.remove(".");
            qFatal("Unable to create backup of the database");
        }
    }

    qDebug("Database backup done");

    open();
}
