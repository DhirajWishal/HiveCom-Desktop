#pragma once

#include <DataLink.hpp>

#include <QThread>
#include <QRandomGenerator64>

/// @brief Reflection data link class.
///	This class simulates an actual physical link in a desktop application without the use of a networking stack.
///	Each instance will have it's own thread and will work via messages sent through the Qt signal/ slot interface.
class ReflectionDataLink : public QThread, public HiveCom::DataLink
{
	Q_OBJECT
public:
	/// @brief Explicit constructor.
	///	@param identifier The current node's identifier.
	///	@param certificate The certificate of the current node.
	///	@param keyPair The key-pair of the certificate and current node.
	explicit ReflectionDataLink(const std::string& identifier, const HiveCom::Certificate& certificate, const HiveCom::Kyber768Key& keyPair);

	/// @brief Default destructor.
	~ReflectionDataLink() override = default;

	/// @brief Send discovery message function.
	///	This function will broadcast the certificate to all the immediate peers.
	void sendDiscovery() override;

	/// @brief Set the peers of the current data link.
	///	@param peers The peers the node can directly communicate with.
	void setPeers(QStringList&& peers);

	/// @brief Get the peers.
	///	@return The peers list.
	[[nodiscard]] const QStringList& getPeers() const { return m_peers; }

signals:
	/// @brief This signal is emitted when the data link is required to send a data packet
	///	to a recipient.
	///	@param identifier The identifier of the peer.
	///	@param content The content to transfer.
	void messageTransmission(std::string identifier, const HiveCom::Bytes& content);

	/// @brief This signal is emitted when a new log should be inserted to the log.
	///	@param content The content to be logged.
	void log(QString content);

public slots:
	/// @brief This slot should be connected in order to communicate with other nodes.
	///	This will handle the incoming message and routing is conducted accordingly.
	///	@param identifier The identifier to send to.
	///	@param content The content that is being transferred.
	void onTransmissionReceived(const std::string& identifier, const HiveCom::Bytes& content);

protected:
	/// @brief Send a message through the networking protocol.
	///	@param receiver The receiver identifier.
	///	@param message The message to send.
	void send(std::string_view receiver, const HiveCom::Bytes& message) override;

	/// @brief Route a message through the network using a routing protocol/ algorithm.
	///	@param receiver The receiver node.
	/// @param message The message to send.
	void route(std::string_view receiver, const HiveCom::Bytes& message) override;

	/// @brief This function is called if a certificate is invalid and if a connection should be blacklisted.
	/// @note This function is called in a thread-safe manner.
	/// @param identifier The identifier to blacklist.
	void blacklistConnection(const std::string& identifier) override;
    
private:
	/// @brief Process a bunch of bytes to be shown by the message logging system.
	///	This will trim the number of bytes being shown to a specified size.
	///	@param bytes The bytes to process.
	///	@return The processed information.
	static QString ProcessBytes(const QByteArray& bytes);

    /// @brief Log a message to the UI.
    /// This will do some formatting when logging everything in.
    /// @param receiver The receiver of the message.
    /// @param message The message that's being transmitted.
    void logMessage(std::string_view receiver, const HiveCom::Bytes& message);

private:
	QStringList m_peers;
	QRandomGenerator64 m_randomGenerator;
};
