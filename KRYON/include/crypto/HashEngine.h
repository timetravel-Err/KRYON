#ifndef KRYON_HASH_ENGINE_H
#define KRYON_HASH_ENGINE_H
#include <openssl/sha.h>
#include <cstring>

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : HashEngine.h
 *
 * Description
 * -----------
 * Centralized hashing service for the KRYON framework.
 *
 * Responsibilities
 * ----------------
 * • Compute hashes from byte arrays
 * • Compute hashes from strings
 * • Verify hash equality
 *
 * Design Notes
 * ------------
 * This is currently a placeholder implementation intended
 * to establish the framework architecture.
 *
 * Future versions may integrate:
 * • SHA-256
 * • SHA3-256
 * • BLAKE3
 * • SM3
 * • Post-Quantum hash algorithms
 *
 * The public interface should remain unchanged.
 * ----------------------------------------------------------
 */

#include "../core/ExperimentConfig.h"
#include "../simulation/SimulationContext.h"
#include "../core/Logger.h"
#include "CryptoTypes.h"

#include <string>

namespace kryon
{

class HashEngine
{
public:

    HashEngine(const ExperimentConfig& config,
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
        Logger::Info("Hash Engine initialized.");
    }

    /* ------------------------------------------------------
     * Compute Hash from ByteArray
     * ------------------------------------------------------*/

   HashValue ComputeHash(const ByteArray& input)
{
    HashValue hash;

    hash.bytes.data.resize(SHA256_DIGEST_LENGTH);

    SHA256(
        input.data.data(),
        input.data.size(),
        hash.bytes.data.data());

    return hash;
}

    /* ------------------------------------------------------
     * Compute Hash from String
     * ------------------------------------------------------*/

 HashValue ComputeHash(const std::string& input)
{
    ByteArray bytes;

    bytes.data.assign(input.begin(), input.end());

    return ComputeHash(bytes);
}
    /* ------------------------------------------------------
     * Verify Hash
     * ------------------------------------------------------*/

    bool VerifyHash(const ByteArray& input,
                    const HashValue& expected)
    {
        HashValue computed = ComputeHash(input);

       return std::memcmp(
           computed.bytes.data.data(),
           expected.bytes.data.data(),
           SHA256_DIGEST_LENGTH) == 0;
    }

    /* ------------------------------------------------------
     * Finalize
     * ------------------------------------------------------*/

    void Finalize()
    {
        Logger::Info("Hash Engine finalized.");
    }

private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;

};

}

#endif
