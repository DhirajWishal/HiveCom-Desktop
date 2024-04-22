#include "ReflectionDataLink.hpp"

ReflectionDataLink::ReflectionDataLink(const std::string& identifier, const HiveCom::Certificate& certificate,
	const HiveCom::Kyber768Key& keyPair)
	: HiveCom::DataLink(identifier, certificate, keyPair)
{
}

void ReflectionDataLink::sendDiscovery()
{
	for (const auto& peer : m_peers)
		emit messageTransmission(peer.toStdString(), HiveCom::ToBytes(createDiscoveryPacket(peer.toStdString())));
}

void ReflectionDataLink::setPeers(QStringList&& peers)
{
	m_peers = std::move(peers);
}

void ReflectionDataLink::onTransmissionReceived(const std::string& identifier, const HiveCom::Bytes& content)
{
	if (m_identifier == identifier)
		onPacketReceived(qobject_cast<ReflectionDataLink*>(sender())->m_identifier, content);
}

void ReflectionDataLink::send(std::string_view receiver, const HiveCom::Bytes& message)
{
	emit messageTransmission(receiver.data(), message);
}

void ReflectionDataLink::route(std::string_view receiver, const HiveCom::Bytes& message)
{
	// If the peer is in our list, send it directly.
	if (m_peers.contains(receiver.data()))
	{
		send(receiver, message);
		return;
	}

	// If the peer is not within our list, then send the message randomly.
	const auto index = m_randomGenerator.bounded(m_peers.size());
	send(m_peers[index].toStdString(), message);
}

void ReflectionDataLink::blacklistConnection(const std::string& identifier)
{
}
