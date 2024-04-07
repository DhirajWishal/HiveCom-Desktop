#include "DesktopDataLink.hpp"

#include <QNetworkDatagram>
#include <QDebug>

constexpr quint16 BroadcastPort = 1234;
constexpr quint16 MessagePort = 1235;

DesktopDataLink::DesktopDataLink(const std::string& identifier, const HiveCom::Certificate& certificate, const HiveCom::Kyber768Key& keyPair)
	: HiveCom::DataLink(identifier, certificate, keyPair)
	, m_pTcpServer(std::make_unique<QTcpServer>(this))
	, m_pUdpSocket(std::make_unique<QUdpSocket>(this))
{
	// Setup the UDP socket.
	connect(m_pUdpSocket.get(), &QUdpSocket::readyRead, this, &DesktopDataLink::onUdpReadyRead);

	if (m_pUdpSocket->bind(QHostAddress::Broadcast, BroadcastPort))
	{
		qDebug() << "UDP socket is up!";
	}
	else
	{
		qDebug() << "UDP socket is not up!";
	}

	// Setup the TCP server.
	connect(m_pTcpServer.get(), &QTcpServer::newConnection, this, &DesktopDataLink::onNewConnectionAvailable);

	if (m_pTcpServer->listen(QHostAddress::Any, MessagePort))
	{
		const auto port = m_pTcpServer->serverPort();
		const auto addr = m_pTcpServer->serverAddress().toString();
		qDebug() << "TCP Server port:" << port << "Address:" << addr;
	}
}

void DesktopDataLink::sendDiscovery()
{
	QUdpSocket* pSocket = new QUdpSocket(this);
	connect(pSocket, &QUdpSocket::error, pSocket, &QObject::deleteLater);
	connect(pSocket, &QUdpSocket::connected, this, [this, pSocket]
		{
			const auto message = ("HiveCom-Desktop; " + QString(m_identifier.c_str())).toUtf8();
			if (pSocket->writeDatagram(message, QHostAddress::Broadcast, BroadcastPort) != message.size())
				qDebug() << "Failed to write the data!";

			pSocket->deleteLater();
		});

	pSocket->connectToHost(QHostAddress::Broadcast, BroadcastPort);
}

void DesktopDataLink::send(std::string_view receiver, const HiveCom::Bytes& message)
{
}

void DesktopDataLink::route(std::string_view receiver, const HiveCom::Bytes& message)
{
}

void DesktopDataLink::onTcpConnected(QString identifier)
{
	qDebug() << "TCP connected!" << identifier;
}

void DesktopDataLink::onTcpDisconnected(QString identifier)
{
	qDebug() << "TCP disconnected!" << identifier;

	m_pTcpSockets[identifier]->deleteLater();
	m_pTcpSockets.remove(identifier);
}

void DesktopDataLink::onTcpReadyRead(QString identifier)
{
	qDebug() << "Ready read!" << identifier;
}

void DesktopDataLink::onUdpConnected()
{
	qDebug() << "Testing";
}

void DesktopDataLink::onUdpDisconnected()
{
	qDebug() << "Testing";
}

void DesktopDataLink::onUdpReadyRead()
{
	const auto datagram = m_pUdpSocket->receiveDatagram();
	const auto data = QString(datagram.data());
	const auto splits = data.split("; ");

	// If we received data with two splits, that means we're probably dealing with a ping packet.
	if (splits.size() == 2)
	{
		const auto& identifier = splits[1];

		// If were the same identifier, then skip.
		if (identifier == QString::fromStdString(m_identifier))
			return;

		// Extract the client type.
		ClientType type;
		const auto& clientType = splits[0];

		if (clientType == "HiveCom-Desktop")
		{
			type = ClientType::Desktop;
		}
		else if (clientType == "HiveCom-Mobile")
		{
			type = ClientType::Mobile;
		}
		else if (clientType == "HiveCom-IoT")
		{
			type = ClientType::Embedded;
		}
		else
		{
			qDebug() << "Invalid client type!";
			return;
		}

		// Emit the signal.
		emit pingReceived(type, identifier);

		// Set up a new connection.
		const auto socket = m_pTcpSockets.insert(identifier, new QTcpSocket(this)).value();
		connect(socket, &QTcpSocket::connected, this, [this, identifier] { onTcpConnected(identifier); });
		connect(socket, &QTcpSocket::disconnected, this, [this, identifier] { onTcpDisconnected(identifier); });
		connect(socket, &QTcpSocket::readyRead, this, [this, identifier] { onTcpReadyRead(identifier); });

		socket->connectToHost(datagram.senderAddress(), MessagePort);
	}
	else
	{
		qDebug() << "Unknown broadcast received.";
	}
}

void DesktopDataLink::onNewConnectionAvailable()
{
	const auto socket = m_pTcpServer->nextPendingConnection();

	// Skip if we received a message from the same client.
	if (socket->peerAddress() == socket->localAddress())
		return;

	qDebug() << "Testing" << socket->peerAddress() << socket->localAddress();
}
