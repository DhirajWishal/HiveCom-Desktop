#include "DesktopDataLink.hpp"
#include "CertificateProvider.hpp"

#include <HiveCom/NetworkManager.hpp>

int main()
{
	const auto [certificate, keyPair] = CertificateProvider::Instance().createCertificate();
	auto manager = HiveCom::NetworkManager(std::make_unique<DesktopDataLink>(certificate, keyPair));
}
