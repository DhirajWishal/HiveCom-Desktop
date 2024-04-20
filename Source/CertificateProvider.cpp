#include "CertificateProvider.hpp"

#include <Core/Kyber768.hpp>
#include <CertificateAuthority.hpp>

#include <QFile>
#include <QDebug>

CertificateProvider::CertificateProvider()
{
	// Load the public key.
	HiveCom::Dilithium3Key::PublicKeyType publicKey;
	QFile publicKeyFile(":/SignKeys/public.DesktopApp-Sign.bin");
	if (publicKeyFile.open(QFile::OpenModeFlag::ReadOnly))
	{
		const auto content = publicKeyFile.readAll();

		if (content.size() == HiveCom::Dilithium3Key::PublicKeySize)
			std::copy(content.begin(), content.end(), publicKey.data());
		else
			qWarning() << "Failed to load the public key file!";
	}

	// Load the private key.
	HiveCom::Dilithium3Key::PrivateKeyType privateKey;
	QFile privateKeyFile(":/SignKeys/private.DesktopApp-Sign.bin");
	if (privateKeyFile.open(QFile::OpenModeFlag::ReadOnly))
	{
		const auto content = privateKeyFile.readAll();

		if (content.size() == HiveCom::Dilithium3Key::PrivateKeySize)
			std::copy(content.begin(), content.end(), privateKey.data());
		else
			qWarning() << "Failed to load the public key file!";
	}

	// Setup the root key and add the public key as trusted.
	m_rootKey = HiveCom::Dilithium3Key(publicKey, privateKey);
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
