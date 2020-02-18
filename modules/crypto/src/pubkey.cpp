#include "crypto/pubkey.h"

#include <QDebug>

#include "crypto/crypto.h"

PubKey::PubKey(QObject *parent, const QByteArray &pub_key, bool compressed)
    : QObject(parent)
    , m_pubkey(pub_key)
    , m_compressed(compressed)
{
    qDebug("Create PubKey");
}

PubKey::~PubKey()
{
    qDebug("Destroy PubKey");
}

const QString PubKey::getAddress() const
{
    // Get hashes and prepend 0x00 as main bitcoin network ident and encode with Base58Check
    return Crypto::base58EncodeCheck(Crypto::ripemd160(Crypto::sha256(m_pubkey)).prepend('\0'));
}

const QByteArray PubKey::getData() const
{
    return m_pubkey;
}
