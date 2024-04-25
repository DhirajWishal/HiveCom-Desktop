#include "ReflectionDataLink.hpp"

ReflectionDataLink::ReflectionDataLink(const std::string& identifier, const HiveCom::Certificate& certificate,
	const HiveCom::Kyber768Key& keyPair)
	: HiveCom::DataLink(identifier, certificate, keyPair)
{
	// Remove this for performance issues.
	// connect(this, &QThread::started, this, [this] {sendDiscovery(); });
}

void ReflectionDataLink::sendDiscovery()
{
	for (const auto& peer : m_peers)
		send(peer.toStdString(), HiveCom::ToBytes(createDiscoveryPacket(peer.toStdString())));
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
	logMessage(receiver, message);
}

void ReflectionDataLink::route(std::string_view receiver, const HiveCom::Bytes& message)
{
	// If the peer is in our list, send it directly.
	if (m_peers.contains(receiver.data()))
	{
		send(receiver, message);
		return;
	}

	// If we only have one peer, send it to it.
	if (m_peers.size() == 1)
	{
		send(m_peers.first().toStdString(), message);
		return;
	}

	// Try to find the previous sender.
	const auto senderDataLink = qobject_cast<ReflectionDataLink*>(sender());
	if (senderDataLink == nullptr)
	{
		send(m_peers[m_randomGenerator.bounded(m_peers.size())].toStdString(), message);
		return;
	}

	// Find who sent the message to us, so we can skip them.
	const auto previousSender = senderDataLink->m_identifier;
	std::string nextReceiver = previousSender;

	// If the peer is not within our list, then send the message randomly.
	while (previousSender == nextReceiver)
		nextReceiver = m_peers[m_randomGenerator.bounded(m_peers.size())].toStdString();

	// Send the message to the next receiver.
	send(nextReceiver, message);
}

void ReflectionDataLink::blacklistConnection(const std::string& identifier)
{
	// Nothing to do here for now.
}

QString ReflectionDataLink::ProcessBytes(const QByteArray& bytes)
{
	constexpr auto MaxCharacterCount = 20;
	if (bytes.size() > MaxCharacterCount)
		return QString("%1 ... %2").arg(bytes.sliced(0, MaxCharacterCount), bytes.sliced(bytes.size() - MaxCharacterCount, MaxCharacterCount));

	return bytes;
}

void ReflectionDataLink::logMessage(std::string_view receiver, const HiveCom::Bytes& message)
{
	const auto content = QByteArray::fromRawData(reinterpret_cast<const char*>(message.data()), message.size());
	const auto splits = content.split('\n');

	QStringList logMessage;
	logMessage << "Message being sent from '" << m_identifier.data() << "' to '" << receiver.data()
		<< "'\nVersion: " << splits[0] << "\n";

	switch (static_cast<HiveCom::MessageFlag>(splits[1].toInt()))
	{
	case HiveCom::MessageFlag::Discovery:
		logMessage << "Flag: Discovery\n";
		logMessage << "Intended receiver: " << splits[2] << "\n";
		logMessage << "Certificate: " << ProcessBytes(splits[3]);
		break;

	case HiveCom::MessageFlag::Authorization:
		logMessage << "Flag: Authorization\n";
		logMessage << "Intended receiver: " << splits[2] << "\n";
		logMessage << "Certificate: " << ProcessBytes(splits[3]) << "\n";
		logMessage << "Ciphertext: " << ProcessBytes(splits[4]);
		break;

	case HiveCom::MessageFlag::Message:
		logMessage << "Flag: Message\n";
		logMessage << "Intended receiver: " << splits[2] << "\n";
		logMessage << "Author (sender): " << splits[3] << "\n";
		logMessage << "Message: " << ProcessBytes(splits[4]);
		break;

	case HiveCom::MessageFlag::Control:
		logMessage << "Flag: Control\n";
		logMessage << "Intended receiver: " << splits[2] << "\n";
		logMessage << "Author (sender): " << splits[3] << "\n";
		logMessage << "Message: " << ProcessBytes(splits[4]);
		break;

	case HiveCom::MessageFlag::Route:
		logMessage << "Flag: Route\n";
		logMessage << "Intended receiver: " << splits[2] << "\n";
		logMessage << "Author (sender): " << splits[3] << "\n";
		logMessage << "Message: " << ProcessBytes(splits[4]);
		break;

	default:
		logMessage << "Packet flag: N/A";
		break;
	}

	logMessage << "\n";
	emit log(logMessage.join(""));
}
