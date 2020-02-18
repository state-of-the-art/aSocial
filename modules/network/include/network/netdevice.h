#ifndef NETDEVICE_H
#define NETDEVICE_H

#include <QObject>

class QTcpSocket;

class NetDevice : public QObject
{
    Q_OBJECT

  public:
    explicit NetDevice(QObject *parent = 0, QString id = "", QString name = "", QString net_address = "",
                       quint16 net_port = 0, quint16 net_broadcast_port = 0, QTcpSocket *socket = NULL);
    ~NetDevice();

    QString id() { return m_id; }
    QString name() { return m_name; }
    QString netAddress() { return m_net_address; }
    quint16 netPort() { return m_net_port; }
    quint16 netBroadcastPort() { return m_net_broadcast_port; }

  private:
    QString m_id;
    QString m_name;
    QString m_net_address;
    quint16 m_net_port;
    quint16 m_net_broadcast_port;

    QTcpSocket *m_socket;
};

#endif  // NETDEVICE_H
