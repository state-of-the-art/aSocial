#include "backend/backend.h"

#include <QStandardPaths>
#include <QTimer>

#include "bedatabase.h"
#include "crypto/crypto.h"
#include "network/network.h"
#include "settings.h"

Backend::Backend(QObject *parent)
    : QObject(parent)
    , m_database(NULL)
    , m_network(NULL)
    , m_broadcast_timer(NULL)
{
    qDebug("Create Backend");

    // Database name & path
    Settings::I()->setDefault("backend/database_name", "public.asc");
    Settings::I()->setDefault("backend/database_path", QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    // In passive mode backend don't send broadcast messages
    Settings::I()->setDefault("backend/broadcast_passive_mode", false);
    // Interval of sending broadcast requests
    Settings::I()->setDefault("backend/broadcast_interval", 60000);
}

Backend::~Backend()
{
    delete m_broadcast_timer;
    delete m_network;
    delete m_database;
    qDebug("Destroy Backend");
}

void Backend::init(const PrivKey *device_passkey)
{
    qDebug("Init Backend");

    initDatabase();

    initDeviceKey(device_passkey);

    initNetwork();
}

void Backend::initDatabase()
{
    qDebug("Init Backend Database");

    m_database = new BEDatabase(this, Settings::I()->setting("backend/database_name").toString(),
                                Settings::I()->setting("backend/database_path").toString());
    m_database->open();
}

void Backend::initDeviceKey(const PrivKey *device_passkey)
{
    qDebug("Init Backend Device key");

    m_device_key = m_database->getDeviceKey();

    if( m_device_key == NULL ) {
        qDebug("Generate new Device key");
        m_device_key = Crypto::I()->genKey();

        m_device_key->encrypt(QString(Crypto::sha3256(device_passkey->getData()).toHex()));

        m_database->storeDeviceKey(m_device_key);
        m_device_key = m_database->getDeviceKey();
    }

    if( m_device_key->isEncrypted() ) {
        m_device_key->decryptForever(QString(Crypto::sha3256(device_passkey->getData()).toHex()));
    }
}

void Backend::initNetwork()
{
    qDebug("Init Backend Networking");

    m_network = new Network(this);
    m_network->init();

    if( ! Settings::I()->setting("backend/broadcast_passive_mode").toBool() ) {
        qDebug("Starting broadcast active mode");
        m_broadcast_timer = new QTimer(this);
        connect(m_broadcast_timer, SIGNAL(timeout()), SLOT(broadcast()));
        broadcast();
        m_broadcast_timer->start(Settings::I()->setting("backend/broadcast_interval").toULongLong());
    }
}

void Backend::broadcast()
{
    m_network->sendBroadcast(QString("Hello from me"), Network::Hello);
}
