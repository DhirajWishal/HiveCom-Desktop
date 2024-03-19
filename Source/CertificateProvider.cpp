#include "CertificateProvider.hpp"

#include <HiveCom/Kyber768.hpp>

CertificateProvider::CertificateProvider()
	: m_rootKey(m_tool.generateKey())
{
}

CertificateProvider& CertificateProvider::Instance()
{
	static CertificateProvider instance;
	return instance;
}

std::pair<HiveCom::Certificate, HiveCom::Kyber768Key> CertificateProvider::createCertificate(const std::string& identifier)
{
	const HiveCom::Kyber768 tool;
	const auto keyPair = tool.generateKey();
	const auto certificate = HiveCom::Certificate(identifier, 0, "0001", 30, "Desktop App", keyPair.getPublicKey(), m_rootKey.getPrivateKey(), m_tool);

	return std::make_pair(certificate, keyPair);
}
