#pragma once

#include <HiveCom/DataLink.hpp>

#include <QNetworkAccessManager>
#include <QTcpServer>
#include <QUdpSocket>

/// @brief Desktop data link class.
class DesktopDataLink final : public QObject, public HiveCom::DataLink  // NOLINT(cppcoreguidelines-special-member-functions)
{
	Q_OBJECT

public:
	/// @brief Client type enum.
	///	This defines all the possible client types that we support.
	enum class ClientType : quint8
	{
		Desktop,	// Client type string: HiveCom-Desktop
		Mobile,		// Client type string: HiveCom-Mobile
		Embedded	// Client type string: HiveCom-IoT
	};

	Q_ENUM(ClientType);

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

signals:
	/// @brief Ping received signal.
	///	This signal is emitted when a new ping is received from someone.
	///	@param type The client type.
	///	@param identifier The client's identifier.
	void pingReceived(ClientType type, QString identifier);

	/// @brief This signal is emitted if a connection has been disconnected.
	///	@param identifier The peer identifier.
	void disconnected(QString identifier);

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
	///	@param identifier The socket identifier.
	void onTcpConnected(const QString& identifier);

	/// @brief This slot is called when the TCP socket is disconnected.
	///	@param identifier The socket identifier.
	void onTcpDisconnected(const QString& identifier);

	/// @brief This slot is called when the TCP socket is ready to read.
	///	@param identifier The socket identifier.
	void onTcpReadyRead(const QString& identifier);

	/// @brief This slot is called when the peer TCP socket is ready to read.
	void onTcpPeerReadyRead();

	/// @brief This slot is called when the UDP socket is connected.
	void onUdpConnected();

	/// @brief This slot is called when the UDP socket is disconnected.
	void onUdpDisconnected();

	/// @brief This slot is called when the UDP socket is ready to read.
	void onUdpReadyRead();

	/// @brief This slot is called when a new connection is available for the TCP server.
	void onNewConnectionAvailable();

private:
	/// @brief Handle a new datagram received signal.
	///	@param pSocket The socket that received the datagram.
	void handleDatagram(QUdpSocket* pSocket);

	/// @brief Create a new network request.
	///	@param address The address to send the request to.
	///	@param path The path of the URL. Default is "/".
	///	@return The network reply pointer.
	QNetworkReply* createNetworkRequest(const QString& address, QString path = "/") const;

private:
	QMap<QString, QTcpSocket*> m_pTcpSockets;
	QMap<QString, QTcpSocket*> m_pPeerTcpSockets;

	std::unique_ptr<QNetworkAccessManager> m_pNetworkAccessManager;
	std::unique_ptr<QTcpServer> m_pTcpServer;
	std::unique_ptr<QUdpSocket> m_pUdpSocket;
};
