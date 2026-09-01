#ifndef KRYON_CRYPTO_ENGINE_H
#define KRYON_CRYPTO_ENGINE_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : CryptoEngine.h
 *
 * Description
 * -----------
 * Central cryptographic facade for the KRYON framework.
 *
 * Responsibilities
 * ----------------
 * • Coordinate all cryptographic services
 * • Expose a unified cryptographic interface
 *
 * Design Notes
 * ------------
 * Other framework components should interact only with
 * CryptoEngine rather than individual cryptographic engines.
 *
 * Future versions may integrate:
 * • OpenSSL
 * • Crypto++
 * • Botan
 * • libsodium
 * • Post-Quantum Cryptography
 * ----------------------------------------------------------
 */

#include "../core/ExperimentConfig.h"
#include "../simulation/SimulationContext.h"
#include "../core/Logger.h"

#include "RandomEngine.h"
#include "HashEngine.h"
#include "KeyGenerator.h"
#include "ECCEngine.h"

namespace kryon
{

class CryptoEngine
{
public:

    CryptoEngine(const ExperimentConfig& config,
                 SimulationContext& context)
        : m_config(config),
          m_context(context),
          m_random(config, context),
          m_hash(config, context),
          m_keyGenerator(config, context),
          m_ecc(config, context)
    {
    }

    /* ------------------------------------------------------
     * Initialize
     * ------------------------------------------------------*/

    void Initialize()
    {
        Logger::Info("Crypto Engine initialized.");

        m_random.Initialize();
        m_hash.Initialize();
        m_keyGenerator.Initialize();
        m_ecc.Initialize();
    }

    /* ------------------------------------------------------
     * Random Numbers
     * ------------------------------------------------------*/

   ByteArray GenerateRandomBytes(std::size_t length)
{
    return m_random.GenerateBytes(length);
}
    /* ------------------------------------------------------
     * Hashing
     * ------------------------------------------------------*/

    HashValue ComputeHash(const ByteArray& input)
    {
        return m_hash.ComputeHash(input);
    }

    HashValue ComputeHash(const std::string& input)
    {
        return m_hash.ComputeHash(input);
    }

    /* ------------------------------------------------------
     * Key Generation
     * ------------------------------------------------------*/

    KeyPair GenerateKeyPair()
    {
        return m_keyGenerator.GenerateKeyPair();
    }

    /* ------------------------------------------------------
     * ECC Operations
     * ------------------------------------------------------*/

    Signature Sign(const ByteArray& message,
                   const PrivateKey& key)
    {
        return m_ecc.Sign(message, key);
    }

    bool Verify(const ByteArray& message,
                const Signature& signature,
                const PublicKey& key)
    {
        return m_ecc.Verify(message, signature, key);
    }

    SharedSecret DeriveSharedSecret(const PrivateKey& privateKey,
                                    const PublicKey& publicKey)
    {
        return m_ecc.DeriveSharedSecret(privateKey, publicKey);
    }
	
	SessionKey DeriveSessionKey(
    const SharedSecret& secret)
{
    return m_ecc.DeriveSessionKey(secret);
}

    /* ------------------------------------------------------
     * Finalize
     * ------------------------------------------------------*/

    void Finalize()
    {
        m_ecc.Finalize();
        m_keyGenerator.Finalize();
        m_hash.Finalize();
        m_random.Finalize();

        Logger::Info("Crypto Engine finalized.");
    }

private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;

    RandomEngine m_random;

    HashEngine m_hash;

    KeyGenerator m_keyGenerator;

    ECCEngine m_ecc;

};

}

#endif
