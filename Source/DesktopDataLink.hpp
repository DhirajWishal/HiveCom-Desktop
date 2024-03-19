#pragma once
#include <HiveCom/DataLink.hpp>

/// @brief Desktop data link class.
class DesktopDataLink final : public HiveCom::DataLink  // NOLINT(cppcoreguidelines-special-member-functions)
{
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
	void send(std::string_view receiver, HiveCom::Bytes message) override;

	/// @brief Route a message through the network using a routing protocol/ algorithm.
	///	@param receiver The receiver node.
	/// @param message The message to send.
	void route(std::string_view receiver, const HiveCom::Bytes& message) override;
};
