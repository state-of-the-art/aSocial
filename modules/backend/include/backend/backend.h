#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>

class QTimer;

class Network;
class BEDatabase;
class PrivKey;

class Backend : public QObject
{
    Q_OBJECT

  public:
    explicit Backend(QObject *parent = 0);
    ~Backend();

    void init(const PrivKey *device_passkey);

  public slots:
    void broadcast();

  private:
    void initDatabase();
    void initDeviceKey(const PrivKey *device_passkey);
    void initNetwork();

    BEDatabase *m_database;
    Network *m_network;
    PrivKey *m_device_key;

    QTimer *m_broadcast_timer;
};

#endif  // BACKEND_H
