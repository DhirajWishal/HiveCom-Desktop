#pragma once

#include <HiveCom/Kyber768Key.hpp>
#include <HiveCom/Certificate.hpp>
#include <HiveCom/Dilithium3.hpp>

/// @brief Certificate provider class.
///	This will generate a new certificate when needed along with it's public and private key pair.
class CertificateProvider final
{
	/// @brief Default constructor.
	CertificateProvider();

	/// @brief Default destructor.
	~CertificateProvider() = default;

public:
	/// @brief Get the instance.
	///	@return The instance reference.
	static CertificateProvider& Instance();

	/// @brief Create a new certificate and the key-pair.
	///	@return The certificate and key-pair.
	[[nodiscard]] std::pair<HiveCom::Certificate, HiveCom::Kyber768Key> createCertificate();

private:
	HiveCom::Dilithium3 m_tool;
	HiveCom::Dilithium3Key m_rootKey;
};
