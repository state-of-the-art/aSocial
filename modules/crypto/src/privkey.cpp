#include "crypto/privkey.h"

#include <QDataStream>
#include <QDebug>

#include "crypto/crypto.h"

PrivKey::PrivKey(QObject *parent, const QByteArray &privkey, bool encrypted, PubKey *pubkey)
    : QObject(parent)
    , m_privkey(privkey)
    , m_encrypted(encrypted)
    , m_pubkey(NULL)
{
    qDebug("Create PrivKey");
    setPubKey(pubkey);
}

PrivKey::~PrivKey()
{
    m_privkey.fill('*');
    qDebug("Destroy PrivKey");
}

void PrivKey::encrypt(const QString &password)
{
    if( m_encrypted ) {
        qWarning("Requested to encrypt already encrypted key");
        return;
    }
    m_privkey = Crypto::passwordEncrypt(Crypto::sha3256(password.toUtf8()), m_privkey);
    m_encrypted = true;
}

bool PrivKey::decrypt(const QString &password, QByteArray &data)
{
    if( ! m_encrypted ) {
        qWarning("Requested to decrypt non-encrypted key");
        data = m_privkey;
    } else {
        if( ! Crypto::passwordDecrypt(Crypto::sha3256(password.toUtf8()), m_privkey, data) )
            return false;
    }

    data.detach();
    return true;
}

bool PrivKey::decryptForever(const QString &password)
{
    if( ! m_encrypted ) {
        qWarning("Requested to decrypt non-encrypted key");
    } else {
        QByteArray data;
        if( ! Crypto::passwordDecrypt(Crypto::sha3256(password.toUtf8()), m_privkey, data) )
            return false;
        m_privkey = data;
        m_encrypted = false;
    }

    return true;
}

void PrivKey::setPubKey(PubKey *pubkey)
{
    m_pubkey = pubkey;
    if( m_pubkey )
        m_pubkey->setParent(this);
}

const QByteArray PrivKey::getData() const
{
    return m_privkey;
}
