#pragma once

#include <HiveCom/DataLink.hpp>

#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>

/// @brief Desktop data link class.
class DesktopDataLink final : public QObject, public HiveCom::DataLink  // NOLINT(cppcoreguidelines-special-member-functions)
{
	Q_OBJECT

public:
	/// @brief Explicit constructor.
	///	@param identifier The current node's identifier.
	///	@param certificate The certificate of the current node.
	///	@param keyPair The key-pair of the certificate and current node.
	explicit DesktopDataLink(const std::string& identifier, const HiveCom::Certificate& certificate, const HiveCom::Kyber768Key& keyPair);

	/// @brief Default destructor.
	~DesktopDataLink() override = default;

	/// @brief Send discovery message function.
	///	This function will broadcast the certificate to all the immediate peers.
	void sendDiscovery() override;

protected:
	/// @brief Send a message through the networking protocol.
	///	@param receiver The receiver identifier.
	///	@param message The message to send.
	void send(std::string_view receiver, const HiveCom::Bytes& message) override;

	/// @brief Route a message through the network using a routing protocol/ algorithm.
	///	@param receiver The receiver node.
	/// @param message The message to send.
	void route(std::string_view receiver, const HiveCom::Bytes& message) override;

private slots:
	/// @brief This slot is called when the TCP socket is connected.
	void onTcpConnected();

	/// @brief This slot is called when the TCP socket is disconnected.
	void onTcpDisconnected();

	/// @brief This slot is called when the TCP socket is ready to read.
	void onTcpReadyRead();

	/// @brief This slot is called when the UDP socket is connected.
	void onUdpConnected();

	/// @brief This slot is called when the UDP socket is disconnected.
	void onUdpDisconnected();

	/// @brief This slot is called when the UDP socket is ready to read.
	void onUdpReadyRead();

	/// @brief This slot is called when a new connection is available for the TCP server.
	void onNewConnectionAvailable();

private:
	std::unique_ptr<QTcpServer> m_pTcpServer;
	std::unique_ptr<QTcpSocket> m_pTcpSocket;
	std::unique_ptr<QUdpSocket> m_pUdpSocket;
};
