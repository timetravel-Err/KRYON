#ifndef KRYON_ECC_ENGINE_H
#define KRYON_ECC_ENGINE_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : ECCEngine.h
 *
 * Description
 * -----------
 * Centralized Elliptic Curve Cryptography (ECC) service.
 *
 * Responsibilities
 * ----------------
 * • Digital signatures
 * • Signature verification
 * • Shared secret derivation
 *
 * Design Notes
 * ------------
 * This is currently a placeholder implementation.
 *
 * Future versions may integrate:
 * • OpenSSL
 * • microECC
 * • Botan
 * • libsodium
 * • Post-Quantum wrappers
 *
 * The public interface should remain unchanged.
 * ----------------------------------------------------------
 */

#include "../core/ExperimentConfig.h"
#include "../simulation/SimulationContext.h"
#include "../core/Logger.h"
#include "CryptoTypes.h"
#include "HashEngine.h"
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/ec.h>
#include <openssl/core_names.h>

namespace kryon
{

class ECCEngine
{
public:

    ECCEngine(const ExperimentConfig& config,
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
        Logger::Info("ECC Engine initialized.");
    }

/* ------------------------------------------------------
 * Sign Message using ECDSA / SHA-256
 * ------------------------------------------------------*/

Signature Sign(const ByteArray& message,
               const PrivateKey& privateKey)
{
    Signature signature;

    if (privateKey.bytes.data.empty())
    {
        Logger::Info(
            "ECDSA signing failed: empty private key.");

        return signature;
    }

    const unsigned char* keyPtr =
        privateKey.bytes.data.data();

    EVP_PKEY* pkey =
        d2i_AutoPrivateKey(
            nullptr,
            &keyPtr,
            privateKey.bytes.data.size());

    if (!pkey)
    {
        Logger::Info(
            "ECDSA signing failed: unable to import private key.");

        return signature;
    }

    EVP_MD_CTX* mdctx =
        EVP_MD_CTX_new();

    if (!mdctx)
    {
        EVP_PKEY_free(pkey);

        Logger::Info(
            "ECDSA signing failed: unable to create digest context.");

        return signature;
    }

    if (EVP_DigestSignInit(
            mdctx,
            nullptr,
            EVP_sha256(),
            nullptr,
            pkey) <= 0)
    {
        Logger::Info(
            "ECDSA signing initialization failed.");

        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);

        return signature;
    }

    if (EVP_DigestSignUpdate(
            mdctx,
            message.data.data(),
            message.data.size()) <= 0)
    {
        Logger::Info(
            "ECDSA signing update failed.");

        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);

        return signature;
    }

    size_t signatureLength = 0;

    if (EVP_DigestSignFinal(
            mdctx,
            nullptr,
            &signatureLength) <= 0)
    {
        Logger::Info(
            "ECDSA signature size determination failed.");

        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);

        return signature;
    }

    signature.bytes.data.resize(signatureLength);

    if (EVP_DigestSignFinal(
            mdctx,
            signature.bytes.data.data(),
            &signatureLength) <= 0)
    {
        signature.bytes.data.clear();

        Logger::Info(
            "ECDSA signature generation failed.");
    }
    else
    {
        signature.bytes.data.resize(signatureLength);

        Logger::Info(
            "ECDSA signature generated.");

        Logger::Info(
            "Signature Size : " +
            std::to_string(signatureLength) +
            " bytes");
    }

    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(pkey);

    return signature;
}


/* ------------------------------------------------------
 * Verify ECDSA Signature using SHA-256
 * ------------------------------------------------------*/

bool Verify(const ByteArray& message,
            const Signature& signature,
            const PublicKey& publicKey)
{
    if (publicKey.bytes.data.empty())
    {
        Logger::Info(
            "ECDSA verification failed: empty public key.");

        return false;
    }

    if (signature.bytes.data.empty())
    {
        Logger::Info(
            "ECDSA verification failed: empty signature.");

        return false;
    }

    const unsigned char* keyPtr =
        publicKey.bytes.data.data();

    EVP_PKEY* pkey =
        d2i_PUBKEY(
            nullptr,
            &keyPtr,
            publicKey.bytes.data.size());

    if (!pkey)
    {
        Logger::Info(
            "ECDSA verification failed: unable to import public key.");

        return false;
    }

    EVP_MD_CTX* mdctx =
        EVP_MD_CTX_new();

    if (!mdctx)
    {
        EVP_PKEY_free(pkey);

        Logger::Info(
            "ECDSA verification failed: unable to create digest context.");

        return false;
    }

    if (EVP_DigestVerifyInit(
            mdctx,
            nullptr,
            EVP_sha256(),
            nullptr,
            pkey) <= 0)
    {
        Logger::Info(
            "ECDSA verification initialization failed.");

        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);

        return false;
    }

    if (EVP_DigestVerifyUpdate(
            mdctx,
            message.data.data(),
            message.data.size()) <= 0)
    {
        Logger::Info(
            "ECDSA verification update failed.");

        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);

        return false;
    }

    int verificationResult =
        EVP_DigestVerifyFinal(
            mdctx,
            signature.bytes.data.data(),
            signature.bytes.data.size());

    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(pkey);

    if (verificationResult == 1)
    {
        Logger::Info(
            "ECDSA signature verification successful.");

        return true;
    }

    if (verificationResult == 0)
    {
        Logger::Info(
            "ECDSA signature verification FAILED.");

        return false;
    }

    Logger::Info(
        "ECDSA signature verification encountered an error.");

    return false;
}
    /* ------------------------------------------------------
     * Derive Shared Secret
     * ------------------------------------------------------*/

  SharedSecret DeriveSharedSecret(
    const PrivateKey& privateKey,
    const PublicKey& publicKey)
{
    SharedSecret secret;

    /* -----------------------------
       Import private key
       ----------------------------- */

    const unsigned char* privPtr =
        privateKey.bytes.data.data();

    EVP_PKEY* priv =
        d2i_AutoPrivateKey(
            nullptr,
            &privPtr,
            privateKey.bytes.data.size());

    if (!priv)
    {
        Logger::Info("Failed to import private key.");
        return secret;
    }

    /* -----------------------------
       Import public key
       ----------------------------- */

    const unsigned char* pubPtr =
        publicKey.bytes.data.data();

    EVP_PKEY* pub =
        d2i_PUBKEY(
            nullptr,
            &pubPtr,
            publicKey.bytes.data.size());

    if (!pub)
    {
        EVP_PKEY_free(priv);

        Logger::Info("Failed to import public key.");

        return secret;
    }

    /* -----------------------------
       Create derive context
       ----------------------------- */

    EVP_PKEY_CTX* ctx =
        EVP_PKEY_CTX_new(priv, nullptr);

    if (!ctx)
    {
        EVP_PKEY_free(priv);
        EVP_PKEY_free(pub);

        Logger::Info("Failed to create ECDH context.");

        return secret;
    }

    if (EVP_PKEY_derive_init(ctx) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(priv);
        EVP_PKEY_free(pub);

        Logger::Info("ECDH initialization failed.");

        return secret;
    }

    if (EVP_PKEY_derive_set_peer(ctx, pub) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(priv);
        EVP_PKEY_free(pub);

        Logger::Info("Failed to set peer key.");

        return secret;
    }

    /* -----------------------------
       Determine secret length
       ----------------------------- */

    size_t secretLen = 0;

    EVP_PKEY_derive(
        ctx,
        nullptr,
        &secretLen);

    secret.bytes.data.resize(secretLen);

    if (EVP_PKEY_derive(
            ctx,
            secret.bytes.data.data(),
            &secretLen) <= 0)
    {
        secret.bytes.data.clear();

        Logger::Info("ECDH derivation failed.");
    }
    else
    {
        secret.bytes.data.resize(secretLen);

        Logger::Info(
            "ECDH shared secret generated.");

        Logger::Info(
            "Shared Secret Size : " +
            std::to_string(secretLen) +
            " bytes");
    }

    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(priv);
    EVP_PKEY_free(pub);

    return secret;
}

/* ------------------------------------------------------
 * Derive Session Key (HKDF-SHA256)
 * ------------------------------------------------------*/

SessionKey DeriveSessionKey(
    const SharedSecret& secret)
{
    SessionKey key;

    ByteArray input;
    input.data = secret.bytes.data;

    HashEngine hash(m_config, m_context);

    HashValue digest =
        hash.ComputeHash(input);

    key.bytes = digest.bytes;

    return key;
}

    /* ------------------------------------------------------
     * Finalize
     * ------------------------------------------------------*/

    void Finalize()
    {
        Logger::Info("ECC Engine finalized.");
    }

private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;

};

}

#endif
