#ifndef NETWORK_H
#define NETWORK_H

#include <QMap>
#include <QObject>

class QUdpSocket;
class QTcpServer;

class NetDevice;

class Network : public QObject
{
    Q_OBJECT

  public:
    enum MessageType { Hello };

    explicit Network(QObject *parent = 0);
    ~Network();

    void init();

    void sendBroadcast(QString data, MessageType type);

  signals:
    void receivedBroadcast(quint8 type, QString data);

  private slots:
    void readUDP();
    void processConnection();

  private:
    void processData(QDataStream &stream);

    QUdpSocket *m_udp_socket;
    QTcpServer *m_tcp_server;

    QMap<QString, NetDevice *> m_netdevices;
};

#endif  // NETWORK_H
