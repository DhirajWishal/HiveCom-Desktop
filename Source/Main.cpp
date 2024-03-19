#include "DesktopDataLink.hpp"
#include "CertificateProvider.hpp"

#include <HiveCom/NetworkManager.hpp>

int main()
{
	const auto [certificate, keyPair] = CertificateProvider::Instance().createCertificate("Desktop-A");
	auto manager = HiveCom::NetworkManager(std::make_unique<DesktopDataLink>("Desktop-A", certificate, keyPair));
}
