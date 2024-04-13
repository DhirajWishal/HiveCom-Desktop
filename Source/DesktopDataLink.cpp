#include "DesktopDataLink.hpp"

#include <QNetworkReply>
#include <QNetworkInterface>
#include <QNetworkDatagram>
#include <QDebug>

constexpr quint16 BroadcastPort = 1234;
constexpr quint16 MessagePort = 1235;

DesktopDataLink::DesktopDataLink(const std::string& identifier, const HiveCom::Certificate& certificate, const HiveCom::Kyber768Key& keyPair)
	: HiveCom::DataLink(identifier, certificate, keyPair)
	, m_pNetworkAccessManager(std::make_unique<QNetworkAccessManager>(this))
	, m_pTcpServer(std::make_unique<QTcpServer>(this))
	, m_pUdpSocket(std::make_unique<QUdpSocket>(this))
{
	// Iterate over all the network interfaces and bind the UDP socket to all the broadcast addresses.
	for (const auto& netInterfaces : QNetworkInterface::allInterfaces())
	{
		if (netInterfaces.flags() & (QNetworkInterface::CanBroadcast | QNetworkInterface::IsRunning))
		{
			for (const auto& address : netInterfaces.addressEntries())
			{
				const auto broadcast = address.broadcast();
				if (broadcast.isNull())
					continue;

				const auto binder = [this](const QHostAddress& address)
					{
						QUdpSocket* pSocket = new QUdpSocket(this);
						if (pSocket->bind(address, BroadcastPort, QAbstractSocket::ReuseAddressHint))
						{
							connect(pSocket, &QUdpSocket::readyRead, this, &DesktopDataLink::onUdpReadyRead);
						}
						else
						{
							qDebug() << "Failed to bind to the address" << address << pSocket->errorString();
							pSocket->deleteLater();
						}
					};

				binder(broadcast);
			}
		}
	}

	// // Setup the UDP socket.
	if (m_pUdpSocket->bind(QHostAddress::AnyIPv4, BroadcastPort, QAbstractSocket::DontShareAddress | QAbstractSocket::ReuseAddressHint))
	{
		connect(m_pUdpSocket.get(), &QUdpSocket::readyRead, this, &DesktopDataLink::onUdpReadyRead);
	}
	else
	{
		qDebug() << "UDP socket is not up!";
		m_pUdpSocket.reset();
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
	// Upon connection, send the discovery packet.
	const auto content = createDiscoveryPacket(identifier.toStdString());
	m_pTcpSockets[identifier]->write(content.data());
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

void DesktopDataLink::onTcpPeerReadyRead()
{
	const auto pSender = qobject_cast<QTcpSocket*>(sender());
	const auto data = pSender->readAll();
	qDebug() << "Peer ready read!" << data;
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
	const auto pSenderSocket = qobject_cast<QUdpSocket*>(sender());
	while (pSenderSocket->hasPendingDatagrams())
		handleDatagram(pSenderSocket);
}

void DesktopDataLink::onNewConnectionAvailable()
{
	const auto pSocket = m_pTcpServer->nextPendingConnection();

	// Skip if we received a message from the same client.
	if (pSocket->peerAddress() == pSocket->localAddress())
		return;

	// Setup the required connections.
	connect(pSocket, &QTcpSocket::readyRead, this, &DesktopDataLink::onTcpPeerReadyRead);
}

void DesktopDataLink::handleDatagram(QUdpSocket* pSocket)
{
	const auto datagram = pSocket->receiveDatagram();
	const auto data = QString(datagram.data());
	const auto splits = data.split("; ");

	// If we received data with two splits, that means we're probably dealing with a ping packet.
	if (splits.size() == 2)
	{
		const auto& identifier = splits[1];

		// If were the same identifier, or it has already been processed, skip.
		if (identifier == QString::fromStdString(m_identifier) || m_pTcpSockets.contains(identifier))
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
		const auto pReply = createNetworkRequest(datagram.senderAddress().toString());
		connect(pReply, &QNetworkReply::finished, this, [this, pReply, identifier, datagram]
			{
				if (pReply->error() == QNetworkReply::NoError)
				{
					const auto pTcpSocket = m_pTcpSockets.insert(identifier, new QTcpSocket(this)).value();
					connect(pTcpSocket, &QTcpSocket::connected, this, [this, identifier] { onTcpConnected(identifier); });
					connect(pTcpSocket, &QTcpSocket::disconnected, this, [this, identifier] { onTcpDisconnected(identifier); });
					connect(pTcpSocket, &QTcpSocket::readyRead, this, [this, identifier] { onTcpReadyRead(identifier); });

					pTcpSocket->connectToHost(datagram.senderAddress(), MessagePort);
				}
				else
				{
					qDebug() << "Error occurred:" << pReply->errorString();
				}

				pReply->deleteLater();
			});
	}
	else
	{
		qDebug() << "Unknown broadcast received.";
	}
}

QNetworkReply* DesktopDataLink::createNetworkRequest(const QString& address) const
{
	auto url = QUrl("http://" + address);
	url.setPort(MessagePort);

	return m_pNetworkAccessManager->get(QNetworkRequest(url));
}
