#include "CertificateProvider.hpp"

#include <Core/Kyber768.hpp>
#include <CertificateAuthority.hpp>

CertificateProvider::CertificateProvider()
{
	m_rootKey = HiveCom::CertificateAuthority::Instance().createKeyPair();
	HiveCom::CertificateAuthority::Instance().addTrustedPublicKey(HiveCom::ToFixedBytes<HiveCom::Dilithium3Key::PublicKeySize>(m_rootKey.getPublicKey()));
}

CertificateProvider& CertificateProvider::Instance()
{
	static CertificateProvider instance;
	return instance;
}

std::pair<HiveCom::Certificate, HiveCom::Kyber768Key> CertificateProvider::createCertificate(const std::string& identifier) const
{
	const HiveCom::Kyber768 tool;
	const auto keyPair = tool.generateKey();
	const auto certificate = HiveCom::CertificateAuthority::Instance().createCertificate(identifier, keyPair.getPublicKey(), m_rootKey.getPrivateKey());

	return std::make_pair(certificate, keyPair);
}
