#include "fedatabase.h"
// Version
#define DATABASE_VERSION 1
// Backward-compatibility with all versions greater than
#define DATABASE_MINIMAL_VERSION 0

#include <QDebug>
#include <QDir>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include "accountdatabase.h"
#include "crypto/crypto.h"
#include "frontend/frontend.h"

FEDatabase::FEDatabase(QObject* parent, QString name, QString path)
    : SqlDatabase(parent, "main.sql", path + "/" + name)
    , m_version(-1)
    , m_active_account(NULL)
{
    qDebug() << "Creating FE Database" << name;
}

FEDatabase::~FEDatabase()
{
    delete m_active_account;
    qDebug("Destroy FE Database");
}

void FEDatabase::open(const QString& password)
{
    SqlDatabase::open(password);

    // Check version
    if( version() == 0 ) {
        qDebug("Start initialiation of the database");
        upgrade(version());
    } else if( version() < DATABASE_MINIMAL_VERSION ) {
        qCritical() << "Version of opened database" << version() << "is too old (minimum supported is"
                    << DATABASE_MINIMAL_VERSION << ")";
        qFatal("Database is too old");
    } else if( version() > DATABASE_VERSION ) {
        qCritical() << "Version of opened database" << version() << "is greater than current client version"
                    << DATABASE_VERSION;
        qFatal("Database is too new");
    } else if( version() < DATABASE_VERSION ) {
        qWarning() << "Your database version" << version() << "will be upgraded to" << DATABASE_VERSION;
        qWarning()
            << "If you find some issues - you always can restore previous version of application and database backup";
        backup();
        upgrade(version());
    }
}

QJsonArray FEDatabase::getAccounts()
{
    qDebug("Requesting accounts list");

    QSqlQuery query(m_db);
    QJsonArray out;
    if( query.exec("SELECT rowid, name, description FROM accounts ORDER BY rowid ASC") ) {
        while( query.next() ) {
            QJsonObject obj;
            obj["id"] = query.value("rowid").toInt();
            obj["name"] = query.value("name").toString();
            obj["description"] = query.value("description").toString();
            out.append(obj);
        }
    } else
        qCritical() << "DB query failed" << version() << query.lastQuery() << query.lastError().text();

    return out;
}

int FEDatabase::createAccount(const QJsonObject& account, const QString& password)
{
    qDebug("Creating new account");

    // Generate new passkey for the account
    PrivKey* passkey = Crypto::I()->genKey();

    if( ! password.isEmpty() )
        passkey->encrypt(password);

    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO accounts (name, description, passkey, passkey_encrypted) VALUES (:name, :description, :passkey, "
        ":passkey_encrypted)");
    query.bindValue(":name", QVariant::fromValue(account.value("name")));
    query.bindValue(":description", QVariant::fromValue(account.value("description")));
    query.bindValue(":passkey", QVariant::fromValue(passkey->getData()));
    query.bindValue(":passkey_encrypted", QVariant::fromValue(! password.isEmpty()));

    if( ! query.exec() )
        qCritical() << "DB query failed" << version() << query.lastQuery() << query.lastError().text();

    // Create database for the new account, if it doesn't exists
    return query.lastInsertId().toInt();
}

bool FEDatabase::openAccount(const int id)
{
    qDebug("Opening account database");

    if( m_active_account != NULL ) {
        qCritical("Active account is opened. Please close account and try again");
        return false;
    }

    QString dbdir = m_path + "/account";
    QString dbfile = QString::number(id).append(".sql");

    QDir(dbdir).mkpath(".");

    // Check & upgrade database
    m_active_account = new AccountDatabase(this, dbfile, dbdir);
    PrivKey* passkey = getAccountPassKey(id);

    QByteArray passkey_d;
    while( ! passkey->decrypt(Frontend::I()->passwordGetWait(qtTrId("Account password")), passkey_d) ) {
    }

    m_active_account->open(passkey_d);
    passkey_d.fill('*');

    return true;
}

void FEDatabase::closeAccount()
{
    delete m_active_account;
    m_active_account = NULL;
}

AccountDatabase* FEDatabase::getCurrentAccount()
{
    if( m_active_account == NULL ) {
        qCritical("No active account present. Please open account and try again");
        return NULL;
    }
    return m_active_account;
}

PrivKey* FEDatabase::getAccountPassKey(const int id)
{
    qDebug("Getting account passkey");

    QSqlQuery query(m_db);

    PrivKey* passkey = new PrivKey(this);

    if( query.exec(QString("SELECT passkey, passkey_encrypted FROM accounts WHERE rowid = %1 LIMIT 1").arg(id)) ) {
        if( query.next() ) {
            delete passkey;
            passkey =
                new PrivKey(this, query.value("passkey").toByteArray(), query.value("passkey_encrypted").toBool());
        }
    } else
        qCritical() << "DB query failed" << version() << query.lastQuery() << query.lastError().text();

    return passkey;
}

qint16 FEDatabase::version()
{
    if( m_version != -1 )
        return m_version;

    qDebug("Get database version");

    // Prepare table before query it
    table("database", QStringList() << "version int not null"
                                    << "description text not null");

    QSqlQuery query(m_db);
    if( query.exec("SELECT version FROM database ORDER BY rowid DESC LIMIT 1") ) {
        query.next();
        if( ! query.isNull(0) ) {
            m_version = query.value(0).toLongLong();
        } else {
            // Database no version - it's empty
            m_version = 0;
        }
    } else
        qCritical() << "DB query failed" << version() << query.lastQuery() << query.lastError().text();

    return m_version;
}

void FEDatabase::setVersion(const qint16 version, const QString& description)
{
    qDebug() << "Set database version" << version;

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO database (version, description) VALUES (:version, :description)");
    query.bindValue(":version", QVariant::fromValue(version));
    query.bindValue(":description", description);

    if( ! query.exec() )
        qCritical() << "DB query failed" << FEDatabase::version() << query.lastQuery() << query.lastError().text();

    m_version = version;
}

void FEDatabase::upgrade(const qint16 from_version)
{
    qDebug() << "Start database upgrade from version" << from_version;

    QSqlQuery query(m_db);

    // Changes in versions should be applied consistently
    switch( from_version + 1 ) {
        case 1:
            table("accounts", QStringList() << "name text not null"
                                            << "description text not null"
                                            << "passkey blob"
                                            << "passkey_encrypted integer");
            setVersion(1, "First version of database");
        default:
            qDebug() << "Upgrade done";
    }
}
