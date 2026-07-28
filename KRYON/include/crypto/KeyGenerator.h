#ifndef KRYON_KEY_GENERATOR_H
#define KRYON_KEY_GENERATOR_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : KeyGenerator.h
 *
 * Description
 * -----------
 * Centralized cryptographic key generation service.
 *
 * Responsibilities
 * ----------------
 * • Generate cryptographic key pairs
 * • Provide a common interface for future algorithms
 *
 * Design Notes
 * ------------
 * This is currently a placeholder implementation.
 *
 * Future versions may support:
 * • ECC Key Generation
 * • RSA Key Generation
 * • Kyber
 * • Dilithium
 * • Falcon
 * • SPHINCS+
 *
 * The public interface should remain unchanged.
 * ----------------------------------------------------------
 */

#include "../core/ExperimentConfig.h"
#include "../simulation/SimulationContext.h"
#include "../core/Logger.h"
#include "CryptoTypes.h"
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/core_names.h>
#include <openssl/param_build.h>
#include <openssl/encoder.h>
#include <memory>
#include <openssl/x509.h>
namespace kryon
{

class KeyGenerator
{
public:

    KeyGenerator(const ExperimentConfig& config,
                 SimulationContext& context)
        : m_config(config),
          m_context(context)
    {
    }

    /* ------------------------------------------------------
     * Initialize
     * ------------------------------------------------------*/

    void Initialize()
    {
        Logger::Info("Key Generator initialized.");
    }

    /* ------------------------------------------------------
     * Generate Key Pair
     * ------------------------------------------------------*/

   KeyPair GenerateKeyPair()
{
    KeyPair keys;

    EVP_PKEY_CTX* ctx =
        EVP_PKEY_CTX_new_from_name(nullptr, "EC", nullptr);

    if (!ctx)
    {
        Logger::Info("Failed to create EVP context.");
        return keys;
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        Logger::Info("Failed to initialize EC key generation.");
        return keys;
    }

    OSSL_PARAM params[2];

    params[0] =
        OSSL_PARAM_construct_utf8_string(
            OSSL_PKEY_PARAM_GROUP_NAME,
            const_cast<char*>("prime256v1"),
            0);

    params[1] =
        OSSL_PARAM_construct_end();

    if (EVP_PKEY_CTX_set_params(ctx, params) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        Logger::Info("Failed to set EC curve.");
        return keys;
    }

    EVP_PKEY* pkey = nullptr;

    if (EVP_PKEY_generate(ctx, &pkey) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        Logger::Info("Key generation failed.");
        return keys;
    }

    EVP_PKEY_CTX_free(ctx);

    /* -------------------------
       Export Public Key (DER)
       ------------------------- */

    unsigned char* pub = nullptr;

    int pubLen =
        i2d_PUBKEY(pkey, &pub);

    if (pubLen > 0)
    {
        keys.publicKey.bytes.data.assign(
            pub,
            pub + pubLen);

        OPENSSL_free(pub);
    }

    /* -------------------------
       Export Private Key (DER)
       ------------------------- */

    unsigned char* priv = nullptr;

    int privLen =
        i2d_PrivateKey(pkey, &priv);

    if (privLen > 0)
    {
        keys.privateKey.bytes.data.assign(
            priv,
            priv + privLen);

        OPENSSL_free(priv);
    }

    EVP_PKEY_free(pkey);

    Logger::Info(
        "Generated real ECC key pair.");
		
		
	Logger::Info(
    "ECC Public Key Size : " +
    std::to_string(keys.publicKey.bytes.data.size()) +
    " bytes");

	Logger::Info(
    "ECC Private Key Size : " +
    std::to_string(keys.privateKey.bytes.data.size()) +
    " bytes");	

    return keys;
}

    /* ------------------------------------------------------
     * Finalize
     * ------------------------------------------------------*/

    void Finalize()
    {
        Logger::Info("Key Generator finalized.");
    }

private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;

};

}

#endif
