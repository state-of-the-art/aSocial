#ifndef CRYPTO_H
#define CRYPTO_H

#include <QObject>

#include "privkey.h"
#include "pubkey.h"

typedef struct ec_group_st EC_GROUP;

class Crypto : public QObject
{
    Q_OBJECT

  public:
    inline static Crypto *I()
    {
        if( s_pInstance == NULL )
            s_pInstance = new Crypto();
        return s_pInstance;
    }
    inline static void destroyI() { delete s_pInstance; }

    void init();

    PrivKey *genKey();

    static bool base58Decode(const QString &b58str, QByteArray &data);
    static bool base58DecodeCheck(const QString &b58str, QByteArray &data);
    static QString base58Encode(const QByteArray &data);
    static QString base58EncodeCheck(const QByteArray &data);

    static QByteArray passwordEncrypt(const QString &password, const QByteArray &data);
    static bool passwordDecrypt(const QString &password, const QByteArray &encrypted_data, QByteArray &decrypted_data);

    static QByteArray ripemd160(const QByteArray &data);
    static QByteArray sha256(const QByteArray &data);
    static QByteArray sha3256(const QByteArray &data);

  private:
    explicit Crypto(QObject *parent = 0);
    ~Crypto();
    static Crypto *s_pInstance;

    EC_GROUP *m_group;
};

#endif  // CRYPTO_H
