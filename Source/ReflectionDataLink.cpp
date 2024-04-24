#include "ReflectionDataLink.hpp"

ReflectionDataLink::ReflectionDataLink(const std::string& identifier, const HiveCom::Certificate& certificate,
                                       const HiveCom::Kyber768Key& keyPair)
: HiveCom::DataLink(identifier, certificate, keyPair)
{
    //	connect(this, &QThread::started, this, [this] {sendDiscovery(); });
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

void ReflectionDataLink::logMessage(std::string_view receiver, const HiveCom::Bytes& message)
{
    constexpr auto MaxCharacterCount = 20;
    
    const auto content = QByteArray::fromRawData(reinterpret_cast<const char*>(message.data()), message.size());
    const auto splits = content.split('\n');
    const auto rawPayload = splits.last();
    
    QString payload;
    if (rawPayload.size() > MaxCharacterCount)
        payload = QString("%1 ... %2").arg(rawPayload.sliced(0, MaxCharacterCount), rawPayload.sliced(rawPayload.size() - MaxCharacterCount, MaxCharacterCount));
    
    else
        payload = rawPayload;
    
    QString messageData;
    for (quint32 i = 0; i < splits.size() - 2; i++)
    {
        QString data;
        
        if (i == 1)
        {
            switch (static_cast<HiveCom::MessageFlag>(splits[i].toInt())) {
                case HiveCom::MessageFlag::Discovery:
                    data = "Discovery";
                    break;
                case HiveCom::MessageFlag::Authorization:
                    data = "Authorization";
                    break;
                case HiveCom::MessageFlag::Message:
                    data = "Message";
                    break;
                case HiveCom::MessageFlag::Control:
                    data = "Control";
                    break;
                case HiveCom::MessageFlag::Route:
                    data = "Route";
                    break;
                default:
                    data = "N/A";
                    break;
            }
        }
        else
        {
            data = splits[i];
        }
        
        messageData += data + '\n';
    }
    
    messageData += payload;
    
    emit log(QString("Message being sent from '%1' to '%2'\nMessage content:\n%3").arg(m_identifier.data(),receiver.data(),messageData));
}
