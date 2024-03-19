#pragma once
#include <HiveCom/DataLink.hpp>

/// @brief Desktop data link class.
class DesktopDataLink final : public HiveCom::DataLink  // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
	DesktopDataLink(const HiveCom::Certificate&, const HiveCom::Kyber768Key& keyPair);
	~DesktopDataLink() override = default;
	void sendDiscovery() override;

protected:
	void send(std::string_view receiver, HiveCom::Bytes message) override;
	void route(std::string_view receiver, const HiveCom::Bytes& message) override;
};
