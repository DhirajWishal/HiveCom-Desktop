#include "ReflectionDataLink.hpp"

ReflectionDataLink::ReflectionDataLink(const std::string& identifier, const HiveCom::Certificate& certificate,
	const HiveCom::Kyber768Key& keyPair)
	: HiveCom::DataLink(identifier, certificate, keyPair)
{
	// Remove this for better performance.
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
	logMessage(receiver, message);
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
	logMessage << "Message being sent from '" << m_identifier.data() << "' to '" << receiver.data() << "'\nMessage content:\n";

	logMessage << "Version: " << splits[0] << "\n";
	switch (static_cast<HiveCom::MessageFlag>(splits[1].toInt())) {
	case HiveCom::MessageFlag::Discovery:
		logMessage << "Flag: Discovery\n";
		logMessage << "Receiver: " << splits[2] << "\n";
		logMessage << "Certificate: " << ProcessBytes(splits[3]) << "\n";
		break;

	case HiveCom::MessageFlag::Authorization:
		logMessage << "Flag: Authorization\n";
		logMessage << "Receiver: " << splits[2] << "\n";
		logMessage << "Certificate: " << ProcessBytes(splits[3]) << "\n";
		logMessage << "Ciphertext: " << ProcessBytes(splits[4]) << "\n";
		break;

	case HiveCom::MessageFlag::Message:
		logMessage << "Flag: Message\n";
		logMessage << "Receiver: " << splits[2] << "\n";
		logMessage << "Sender: " << splits[3] << "\n";
		logMessage << "Message: " << ProcessBytes(splits[4]) << "\n";
		break;

	case HiveCom::MessageFlag::Control:
		logMessage << "Flag: Control\n";
		logMessage << "Receiver: " << splits[2] << "\n";
		logMessage << "Sender: " << splits[3] << "\n";
		logMessage << "Message: " << ProcessBytes(splits[4]) << "\n";
		break;

	case HiveCom::MessageFlag::Route:
		logMessage << "Flag: Route\n";
		logMessage << "Receiver: " << splits[2] << "\n";
		logMessage << "Sender: " << splits[3] << "\n";
		logMessage << "Message: " << ProcessBytes(splits[4]) << "\n";
		break;

	default:
		logMessage << "Packet flag: N/A\n";
		break;
	}

	emit log(logMessage.join(""));
}
