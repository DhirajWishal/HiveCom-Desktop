#pragma once

#include <Core/Kyber768Key.hpp>
#include <Core/Certificate.hpp>

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
	///	@param identifier The certificate identifier.
	///	@return The certificate and key-pair.
	[[nodiscard]] std::pair<HiveCom::Certificate, HiveCom::Kyber768Key> createCertificate(const std::string& identifier) const;

private:
	HiveCom::Dilithium3Key m_rootKey;
};
