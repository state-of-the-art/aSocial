#include "network/netdevice.h"

#include <QDebug>
#include <QHostAddress>
#include <QTcpSocket>

NetDevice::NetDevice(QObject *parent, QString id, QString name, QString net_address, quint16 net_port,
                     quint16 net_broadcast_port, QTcpSocket *socket)
    : QObject(parent)
    , m_id(id)
    , m_name(name)
    , m_net_address(net_address)
    , m_net_port(net_port)
    , m_net_broadcast_port(net_broadcast_port)
    , m_socket(socket)
{
    qDebug() << "Creating NetDevice" << m_id << m_name;

    // Connecting to the device
    if( m_socket == NULL ) {
        m_socket = new QTcpSocket(this);
        m_socket->connectToHost(QHostAddress(m_net_address), m_net_port);
    }
}

NetDevice::~NetDevice()
{
    delete m_socket;
    qDebug() << "Destroy NetDevice" << m_id << m_name;
}
