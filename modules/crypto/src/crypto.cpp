#include "crypto/crypto.h"

#include <openssl/crypto.h>
#include <openssl/ripemd.h>
#include <openssl/ssl.h>

#include <QCryptographicHash>
#include <QDebug>
#include <ctime>

#include "simpleqtcryptor.h"

Crypto *Crypto::s_pInstance = NULL;

#define CRYPTO_CURVE NID_secp256k1
#define CRYPTO_CIPHER EVP_aes_256_cbc()
#define CRYPTO_HASHER EVP_sha256()

Crypto::Crypto(QObject *parent)
    : QObject(parent)
    , m_group(NULL)
{
    qDebug("Create Crypto");
    init();
}

Crypto::~Crypto()
{
    EC_GROUP_free(m_group);
    qDebug("Destroy Crypto");
}

void Crypto::init()
{
    SSL_library_init();
    SSL_load_error_strings();

    // Init eliptic group
    m_group = EC_GROUP_new_by_curve_name(CRYPTO_CURVE);
    EC_GROUP_precompute_mult(m_group, NULL);
    EC_GROUP_set_point_conversion_form(m_group, POINT_CONVERSION_COMPRESSED);

    // Randomization
    srand(std::time(NULL));
}

PrivKey *Crypto::genKey()
{
    EC_KEY *key = NULL;
    EC_GROUP *group = NULL;

    PubKey *pubkey = NULL;
    PrivKey *privkey = NULL;

    // Generate new key
    key = EC_KEY_new();
    group = EC_GROUP_dup(m_group);
    EC_KEY_set_group(key, group);
    EC_KEY_generate_key(key);

    // Get private key
    privkey = new PrivKey(this, QByteArray::fromHex(BN_bn2hex(EC_KEY_get0_private_key(key))), false, NULL);

    // Get public key
    pubkey = new PubKey(
        privkey,
        QByteArray::fromHex(EC_POINT_point2hex(group, EC_KEY_get0_public_key(key), POINT_CONVERSION_COMPRESSED, NULL)),
        true);

    // Free key
    EC_KEY_free(key);
    EC_GROUP_free(group);

    // Set public key
    privkey->setPubKey(pubkey);

    return privkey;
}

QByteArray Crypto::passwordEncrypt(const QString &password, const QByteArray &data)
{
    QByteArray encrypted_data;
    QSharedPointer<SimpleQtCryptor::Key> key(new SimpleQtCryptor::Key(password));
    SimpleQtCryptor::Encryptor e(key, SimpleQtCryptor::SERPENT_32, SimpleQtCryptor::ModeCFB,
                                 SimpleQtCryptor::NoChecksum);

    if( e.encrypt(data, encrypted_data, true) != SimpleQtCryptor::NoError )
        qFatal("Unable to encrypt data with password");
    return encrypted_data;
}

bool Crypto::passwordDecrypt(const QString &password, const QByteArray &encrypted_data, QByteArray &decrypted_data)
{
    QSharedPointer<SimpleQtCryptor::Key> key(new SimpleQtCryptor::Key(password));
    SimpleQtCryptor::Decryptor d(key, SimpleQtCryptor::SERPENT_32, SimpleQtCryptor::ModeCFB);

    if( d.decrypt(encrypted_data, decrypted_data, true) != SimpleQtCryptor::NoError ) {
        qWarning("Unable to decrypt data with password");
        return false;
    }
    return true;
}

QByteArray Crypto::ripemd160(const QByteArray &data)
{
    return QByteArray(
        reinterpret_cast<char *>(RIPEMD160(reinterpret_cast<const uchar *>(data.constData()), data.size(), NULL)));
}

QByteArray Crypto::sha256(const QByteArray &data)
{
    return QCryptographicHash::hash(data, QCryptographicHash::Sha256);
}

QByteArray Crypto::sha3256(const QByteArray &data)
{
    return QCryptographicHash::hash(data, QCryptographicHash::Sha3_256);
}

/**
 * This code from the Bitcoin core, rewrited to the Qt data types
 * files: base58.c, base58.h
 */
// Copyright (c) 2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

/* All alphanumeric characters except for "0", "I", "O", and "l" */
static const char *abc_base58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

bool Crypto::base58Decode(const QString &b58str, QByteArray &data)
{
    const char *psz = b58str.toLatin1().constData();
    // Skip leading spaces.
    while( *psz && isspace(*psz) ) psz++;
    // Skip and count leading '1's.
    int zeroes = 0;
    while( *psz == '1' ) {
        zeroes++;
        psz++;
    }
    // Allocate enough space in big-endian base256 representation.
    std::vector<unsigned char> b256(strlen(psz) * 733 / 1000 + 1);  // log(58) / log(256), rounded up.
    // Process the characters.
    while( *psz && ! isspace(*psz) ) {
        // Decode base58 character
        const char *ch = strchr(abc_base58, *psz);
        if( ch == NULL )
            return false;
        // Apply "b256 = b256 * 58 + ch".
        int carry = ch - abc_base58;
        for( std::vector<unsigned char>::reverse_iterator it = b256.rbegin(); it != b256.rend(); it++ ) {
            carry += 58 * (*it);
            *it = carry % 256;
            carry /= 256;
        }
        Q_ASSERT(carry == 0);
        psz++;
    }
    // Skip trailing spaces.
    while( isspace(*psz) ) psz++;
    if( *psz != 0 )
        return false;
    // Skip leading zeroes in b256.
    std::vector<unsigned char>::iterator it = b256.begin();
    while( it != b256.end() && *it == 0 ) it++;
    // Copy result into output vector.
    data.resize(zeroes + (b256.end() - it));
    data.fill(0x00, zeroes);
    while( it != b256.end() ) data.append(*(it++));
    return true;
}

bool Crypto::base58DecodeCheck(const QString &b58str, QByteArray &data)
{
    if( ! base58Decode(b58str.toLatin1().constData(), data) ) {
        qDebug("Unable to decode b58 string");
        data.clear();
        return false;
    }

    if( data.size() < 4 ) {
        qDebug("Data size less than 4 bytes");
        data.clear();
        return false;
    }

    // re-calculate the checksum, insure it matches the included 4-byte checksum
    QByteArray hash = sha256(sha256(data.left(data.size() - 4)));
    if( hash.left(4) != data.right(4) ) {
        qDebug() << "Hash of data and predefined hash not match" << hash.left(4).toHex() << data.right(4).toHex();
        data.clear();
        return false;
    }
    data.chop(4);

    return true;
}

QString Crypto::base58Encode(const QByteArray &data)
{
    const unsigned char *pbegin = (uchar *)(data.data());
    const unsigned char *pend = (uchar *)(data.data() + data.size());
    // Skip & count leading zeroes.
    int zeroes = 0;
    while( pbegin != pend && *pbegin == 0 ) {
        pbegin++;
        zeroes++;
    }
    // Allocate enough space in big-endian base58 representation.
    std::vector<unsigned char> b58((pend - pbegin) * 138 / 100 + 1);  // log(256) / log(58), rounded up.
    // Process the bytes.
    while( pbegin != pend ) {
        int carry = *pbegin;
        // Apply "b58 = b58 * 256 + ch".
        for( std::vector<unsigned char>::reverse_iterator it = b58.rbegin(); it != b58.rend(); it++ ) {
            carry += 256 * (*it);
            *it = carry % 58;
            carry /= 58;
        }
        Q_ASSERT(carry == 0);
        pbegin++;
    }
    // Skip leading zeroes in base58 result.
    std::vector<unsigned char>::iterator it = b58.begin();
    while( it != b58.end() && *it == 0 ) it++;
    // Translate the result into a string.
    QString b58str;
    b58str.fill('1', zeroes);
    while( it != b58.end() ) b58str += abc_base58[*(it++)];
    return b58str;
}

QString Crypto::base58EncodeCheck(const QByteArray &data)
{
    // Add 4-byte hash check to the end of data
    QByteArray process(data);
    QByteArray hash = sha256(sha256(data));
    process.append(hash.left(4));

    return base58Encode(process);
}
