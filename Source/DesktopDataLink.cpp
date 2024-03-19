#include "DesktopDataLink.hpp"

DesktopDataLink::DesktopDataLink(const HiveCom::Certificate& certificate, const HiveCom::Kyber768Key& keyPair)
	: HiveCom::DataLink("Desktop-App", certificate, keyPair)
{
}

void DesktopDataLink::sendDiscovery()
{
}

void DesktopDataLink::send(std::string_view receiver, HiveCom::Bytes message)
{
}

void DesktopDataLink::route(std::string_view receiver, const HiveCom::Bytes& message)
{
}