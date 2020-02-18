#ifndef PRIVKEY_H
#define PRIVKEY_H

#include <QObject>

#include "pubkey.h"

class PrivKey : public QObject
{
    Q_OBJECT

  public:
    explicit PrivKey(QObject *parent = 0, const QByteArray &privkey = "", bool encrypted = false,
                     PubKey *pubkey = NULL);
    ~PrivKey();

    // Returns current encrypted state
    inline bool isEncrypted() const { return m_encrypted; }

    // Check privkey data
    inline bool isValid() const { return m_privkey.size() > 0; }

    // Encrypt private key and set encrypted state
    void encrypt(const QString &password);

    // Returns decrypted private key
    bool decrypt(const QString &password, QByteArray &data);

    // Decrypt private key and set decrypted state
    bool decryptForever(const QString &password);

    void setPubKey(PubKey *pubkey);
    const PubKey *getPubKey() const { return m_pubkey; }
    const QByteArray getData() const;

  private:
    QByteArray m_privkey;
    bool m_encrypted;
    PubKey *m_pubkey;
};

#endif  // PRIVKEY_H
