#ifndef KRYON_RANDOM_ENGINE_H
#define KRYON_RANDOM_ENGINE_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : RandomEngine.h
 *
 * Description
 * -----------
 * Centralized random number generation service for the
 * KRYON framework.
 *
 * Responsibilities
 * ----------------
 * • Generate random byte arrays
 * • Generate cryptographic nonces
 * • Provide reproducible randomness for simulations
 *
 * Design Notes
 * ------------
 * This engine currently relies on ns-3's random number
 * generators to ensure deterministic and reproducible
 * simulation experiments.
 *
 * Future versions may support true cryptographic random
 * generators through OpenSSL, Botan, libsodium, etc.
 * ----------------------------------------------------------
 */

#include "../core/ExperimentConfig.h"
#include "../simulation/SimulationContext.h"
#include "../core/Logger.h"
#include "CryptoTypes.h"

#include "ns3/random-variable-stream.h"

namespace kryon
{

class RandomEngine
{
public:

    RandomEngine(const ExperimentConfig& config,
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
        Logger::Info("Random Engine initialized.");
    }

    /* ------------------------------------------------------
     * Generate Random Byte Array
     * ------------------------------------------------------*/

    ByteArray GenerateBytes(std::size_t length)
    {
        ByteArray result;

        ns3::Ptr<ns3::UniformRandomVariable> rng =
            ns3::CreateObject<ns3::UniformRandomVariable>();

        for (std::size_t i = 0; i < length; ++i)
        {
            result.data.push_back(
                static_cast<uint8_t>(
                    rng->GetInteger(0,255)));
        }

        return result;
    }

    /* ------------------------------------------------------
     * Generate Nonce
     * ------------------------------------------------------*/

    Nonce GenerateNonce(std::size_t length = 16)
    {
        Nonce nonce;
        nonce.bytes = GenerateBytes(length);
        return nonce;
    }
	
	
	/* ------------------------------------------------------
	 * Generate Random Time
	 * ------------------------------------------------------*/

	double GenerateUniformDouble(
		double minimum,
		double maximum)
	{
		ns3::Ptr<ns3::UniformRandomVariable> rng =
			ns3::CreateObject<ns3::UniformRandomVariable>();

		return rng->GetValue(
			minimum,
			maximum);
	}

    /* ------------------------------------------------------
     * Finalize
     * ------------------------------------------------------*/

    void Finalize()
    {
        Logger::Info("Random Engine finalized.");
    }

private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;

};

}

#endif
