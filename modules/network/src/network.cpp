#include "network/network.h"

// Version of protocol
#define NETWORK_PROTOCOL_VERSION 0x0001
// Backward-compatibility with all versions greater than
#define NETWORK_MINIMAL_PROTOCOL_VERSION 0x0001

#include <QDataStream>
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>

#include "network/netdevice.h"
#include "settings.h"

Network::Network(QObject *parent)
    : QObject(parent)
    , m_udp_socket(new QUdpSocket(this))
    , m_tcp_server(new QTcpServer(this))
{
    qDebug("Create Network");

    // TCP & UDP target port
    Settings::I()->setDefault("network/port", 49100);
    // TCP & UDP bind host (leave empty to listen both IPv4 & IPv6)
    Settings::I()->setDefault("network/listen_host", "");
    // TCP & UDP bind port
    Settings::I()->setDefault("network/listen_port", Settings::I()->setting("network/port").toUInt());
}

Network::~Network()
{
    delete m_tcp_server;
    delete m_udp_socket;
    qDebug("Destroy Network");
}

void Network::init()
{
    qDebug("Init Network");

    QString bind_host = Settings::I()->setting("network/listen_host").toString();
    quint16 bind_port = Settings::I()->setting("network/listen_port").toUInt();
    qDebug() << "Binging UDP" << bind_host << bind_port;
    if( m_udp_socket->bind(bind_host.isEmpty() ? QHostAddress::Any : QHostAddress(bind_host), bind_port) ) {
        connect(m_udp_socket, SIGNAL(readyRead()), SLOT(readUDP()));
    } else
        qCritical() << "I can't hear broadcast messages because unable to bind UDP host:" << bind_host
                    << "port:" << bind_port;

    qDebug() << "Binding TCP" << bind_host << bind_port;
    if( m_tcp_server->listen(bind_host.isEmpty() ? QHostAddress::Any : QHostAddress(bind_host), bind_port) ) {
        connect(m_tcp_server, SIGNAL(newConnection()), SLOT(processConnection()));
    } else
        qCritical() << "I can't hear broadcast messages because unable to bind UDP host:" << bind_host
                    << "port:" << bind_port;
}

void Network::sendBroadcast(QString data, MessageType type)
{
    qDebug() << "Sending UDP message:" << data;
    QByteArray bytes;
    QDataStream out(&bytes, QIODevice::WriteOnly);

    out << quint64(0);  // Data size
    out << quint8(type);
    out << quint16(NETWORK_PROTOCOL_VERSION);
    quint64 crc_pos = out.device()->pos();
    out << quint16(0);  // CRC
    out << data;
    out.device()->seek(quint64(0));
    out << quint64(bytes.size() - sizeof(quint64));
    out.device()->seek(crc_pos);
    out << quint16(qChecksum(bytes.constData(), bytes.size()));

    m_udp_socket->writeDatagram(bytes, QHostAddress::Broadcast, Settings::I()->setting("network/port").toUInt());
}

void Network::readUDP()
{
    while( m_udp_socket->hasPendingDatagrams() ) {
        QByteArray datagram;
        datagram.resize(m_udp_socket->pendingDatagramSize());
        QHostAddress sender;
        quint16 sender_port;

        m_udp_socket->readDatagram(datagram.data(), datagram.size(), &sender, &sender_port);

        QString address_port = sender.toString().append(" %1").arg(sender_port);
        qDebug() << "Incoming UDP datagram from" << address_port;

        QDataStream stream(&datagram, QIODevice::ReadWrite);

        // Check datagram size
        quint64 size = 0;
        if( quint64(stream.device()->size()) <= sizeof(quint64) ) {
            qDebug() << "Skipping datagram. Is too small" << stream.device()->size();
            continue;
        }
        stream >> size;
        if( stream.device()->size() - sizeof(quint64) < size ) {
            qDebug() << "Skipping datagram. Size invalid" << stream.device()->size();
            continue;
        }

        quint8 type = 0;
        stream >> type;

        if( type == Network::Hello ) {
            // Create network device object
            if( ! m_netdevices.contains(address_port) )
                m_netdevices[address_port] = new NetDevice(this, address_port, "", sender.toString(), 0, sender_port);
            else
                continue;
        }

        // Check version of protocol
        quint16 version;
        stream >> version;
        if( version > NETWORK_PROTOCOL_VERSION ) {
            qWarning() << "Skipping datagram. Version of protocol" << version << "is greater than supported"
                       << NETWORK_PROTOCOL_VERSION;
            return;
        } else if( version < NETWORK_MINIMAL_PROTOCOL_VERSION ) {
            qWarning() << "Skipping datagram. Version of protocol" << version << "is lower than supported"
                       << NETWORK_MINIMAL_PROTOCOL_VERSION;
            return;
        }

        // Processing datagram

        // Check CRC
        quint16 crc = 0;
        stream >> crc;
        stream.device()->seek(stream.device()->pos() - sizeof(quint16));
        stream << quint16(0);
        if( qChecksum(datagram.constData(), datagram.size()) != crc ) {
            qDebug() << "Skipping datagram. CRC is invalid" << crc << qChecksum(datagram.constData(), datagram.size());
            continue;
        }

        switch( type ) {
            case Hello: {
                // Processing data
                QString data;
                stream >> data;
                qDebug() << "Received UDP message:" << data;
                emit receivedBroadcast(type, data);
                break;
            }
            default: {
                qWarning() << "Skipping datagram. Unknown data type" << type;
                break;
            }
        }
    }
}

void Network::processConnection()
{
    QTcpSocket *socket = m_tcp_server->nextPendingConnection();
    QString address_port = socket->peerAddress().toString().append(" %1").arg(socket->peerPort());
    qDebug() << "Incoming TCP connection from" << address_port << socket->peerName();
    socket->close();
}

void Network::processData(QDataStream &stream)
{
}
