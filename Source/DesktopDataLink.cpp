#include "DesktopDataLink.hpp"

#include <QNetworkReply>
#include <QNetworkDatagram>
#include <QDebug>

#include "CertificateAuthority.hpp"

constexpr quint16 BroadcastPort = 1234;
constexpr quint16 MessagePort = 1235;

DesktopDataLink::DesktopDataLink(const std::string& identifier, const HiveCom::Certificate& certificate, const HiveCom::Kyber768Key& keyPair)
	: HiveCom::DataLink(identifier, certificate, keyPair)
	, m_pNetworkAccessManager(std::make_unique<QNetworkAccessManager>(this))
	, m_pHttpServer(std::make_unique<QHttpServer>(this))
	, m_pUdpSocket(std::make_unique<QUdpSocket>(this))
{
	// Set the on authentication callback.
	setOnAuthenticatedCallback([this](const std::string& identifier) {emit pingReceived(ClientType::Desktop, QString::fromStdString(identifier)); });

	// Setup the UDP socket.
	if (m_pUdpSocket->bind(QHostAddress::AnyIPv4, BroadcastPort, QAbstractSocket::DontShareAddress | QAbstractSocket::ReuseAddressHint))
	{
		connect(m_pUdpSocket.get(), &QUdpSocket::readyRead, this, &DesktopDataLink::onUdpReadyRead);
	}
	else
	{
		qDebug() << "UDP socket is not up!";
		m_pUdpSocket.reset();
	}

	// Setup the HTTP server.
	m_pHttpServer->route("/ping", [this](const QHttpServerRequest& request) {return onPingReceived(request); });
	m_pHttpServer->route("/discovery", [this](const QHttpServerRequest& request) {return onDiscoveryReceived(request); });
	m_pHttpServer->route("/message", [this](const QHttpServerRequest& request) {return onMessageReceived(request); });

	if (!m_pHttpServer->listen(QHostAddress::Any, MessagePort))
	{
		qDebug() << "Failed to create the HTTP server!";
	}
}

void DesktopDataLink::sendDiscovery()
{
	QUdpSocket* pSocket = new QUdpSocket(this);
	connect(pSocket, &QUdpSocket::connected, this, [this, pSocket]
		{
			const auto message = ("HiveCom-Desktop; " + QString::fromStdString(m_identifier).toUtf8());
			if (pSocket->writeDatagram(message, QHostAddress::Broadcast, BroadcastPort) != message.size())
				qDebug() << "Failed to write the data!";

			pSocket->deleteLater();
		});

	pSocket->connectToHost(QHostAddress::Broadcast, BroadcastPort);
}

void DesktopDataLink::send(std::string_view receiver, const HiveCom::Bytes& message)
{
	if (m_identifierHostAddressMap.contains(receiver.data()))
		sendNetworkRequest(HostAddressToIP(m_identifierHostAddressMap[receiver.data()]), "/message", QByteArray::fromRawData(reinterpret_cast<const char*>(message.data()), message.size()));

	// TODO: Handle reply stuff.
}

void DesktopDataLink::route(std::string_view receiver, const HiveCom::Bytes& message)
{
	// TODO: Implement a routing protocol.
	send(receiver, message);
}

void DesktopDataLink::blacklistConnection(const std::string& identifier)
{
}

void DesktopDataLink::onUdpReadyRead() const
{
	const auto pSenderSocket = qobject_cast<QUdpSocket*>(sender());
	while (pSenderSocket->hasPendingDatagrams())
		handleDatagram(pSenderSocket);
}

const char* DesktopDataLink::onPingReceived(const QHttpServerRequest& request) const
{
	sendNetworkRequest(HostAddressToIP(request.remoteAddress()), "/discovery", QByteArray::fromStdString(createDiscoveryPacket(request.body().toStdString())));
	return "OK";
}

const char* DesktopDataLink::onDiscoveryReceived(const QHttpServerRequest& request)
{
	const auto rawCertificate = request.body();
	const auto certificate = HiveCom::Certificate(rawCertificate.toStdString());

	// If were the same identifier, or it has already been processed, skip.
	if (certificate.getIdentifier() == m_identifier)
		return "Not Ok";

	onPacketReceived(certificate.getIdentifier().data(), HiveCom::ToBytes(rawCertificate.toStdString()));
	m_identifierHostAddressMap[certificate.getIdentifier().data()] = request.remoteAddress();

	// Emit the signal.
	emit pingReceived(ClientType::Desktop, QString::fromStdString(certificate.getIdentifier().data()));
	return "OK";
}

const char* DesktopDataLink::onMessageReceived(const QHttpServerRequest& request)
{
	if (m_identifierHostAddressMap.values().contains(request.remoteAddress()))
		return "BAD REQUEST";

	onPacketReceived(m_identifierHostAddressMap.key(request.remoteAddress()), HiveCom::ToBytes(request.body().data()));
	return "Ok";
}

void DesktopDataLink::handleDatagram(QUdpSocket* pSocket) const
{
	const auto datagram = pSocket->receiveDatagram();
	const auto data = QString(datagram.data());
	const auto splits = data.split("; ");

	// If we received data with two splits, that means we're probably dealing with a ping packet.
	if (splits.size() == 2)
	{
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

		sendNetworkRequest(datagram.senderAddress().toString(), "/ping", QByteArray::fromStdString(m_identifier));
	}
	else
	{
		qDebug() << "Unknown broadcast received.";
	}
}

QNetworkReply* DesktopDataLink::createNetworkRequest(const QString& address, const QString& path /*= "/ping"*/, const QByteArray& content /*= ""*/) const
{
	auto url = QUrl("http://" + address);
	url.setPort(MessagePort);
	url.setPath(path);
	qDebug() << url;

	QNetworkRequest request(url);

	if (content.isEmpty())
		return m_pNetworkAccessManager->get(request);

	request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "text/plain");
	return m_pNetworkAccessManager->post(request, content);
}

void DesktopDataLink::sendNetworkRequest(const QString& address, const QString& path, const QByteArray& content) const
{
	const auto pReply = createNetworkRequest(address, path, content);
	connect(pReply, &QNetworkReply::finished, this, [pReply] { pReply->deleteLater(); });
}

QString DesktopDataLink::HostAddressToIP(const QHostAddress& address)
{
	const auto addressString = address.toString();

	if (addressString.contains("::"))
		return addressString.split(':').last();

	return addressString;
}
