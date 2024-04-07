#include "DesktopDataLink.hpp"

#include <QtConcurrent/QtConcurrent>
#include <QDebug>

constexpr quint16 BroadcastPort = 1234;

DesktopDataLink::DesktopDataLink(const std::string& identifier, const HiveCom::Certificate& certificate, const HiveCom::Kyber768Key& keyPair)
	: HiveCom::DataLink(identifier, certificate, keyPair)
	, m_pTcpServer(std::make_unique<QTcpServer>())
	// , m_pTcpSocket(std::make_unique<QTcpSocket>())
	, m_pUdpSocket(std::make_unique<QUdpSocket>())
{
	connect(m_pUdpSocket.get(), &QUdpSocket::readyRead, this, &DesktopDataLink::onUdpReadyRead);
	if (m_pUdpSocket->bind(QHostAddress::Broadcast, BroadcastPort))
	{
		qDebug() << "UDP socket is up!";
	}
	else
	{
		qDebug() << "UDP socket is not up!";
	}

	connect(m_pTcpServer.get(), &QTcpServer::newConnection, this, &DesktopDataLink::onNewConnectionAvailable);
	connect(m_pTcpServer.get(), &QTcpServer::newConnection, this, &DesktopDataLink::onNewConnectionAvailable);

	if (m_pTcpServer->listen(QHostAddress::LocalHost, BroadcastPort))
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
			if (pSocket->writeDatagram(message, QHostAddress::Broadcast, BroadcastPort) < 0)
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

void DesktopDataLink::onTcpConnected()
{
	qDebug() << "Testing";
}

void DesktopDataLink::onTcpDisconnected()
{
	qDebug() << "Testing";
}

void DesktopDataLink::onTcpReadyRead()
{
	m_pTcpSocket->readAll();
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
	const auto bytes = m_pUdpSocket->readAll();
}

void DesktopDataLink::onNewConnectionAvailable()
{
	qDebug() << "Testing";
}
