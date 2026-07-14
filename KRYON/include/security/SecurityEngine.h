#ifndef KRYON_SECURITY_ENGINE_H
#define KRYON_SECURITY_ENGINE_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : SecurityEngine.h
 *
 * Description
 * -----------
 * Responsible for all security-related functionality.
 *
 * Future Modules
 * --------------
 * • Authentication
 * • Key Management
 * • Trust Computation
 * • ECC
 * • Hashing
 * • PUF
 * • Blockchain
 * • DID / VC
 * • Zero Knowledge Proofs
 * ----------------------------------------------------------
 */

#include "../core/ExperimentConfig.h"
#include "../simulation/SimulationContext.h"
#include "../core/Logger.h"
namespace kryon
{

class SecurityEngine
{
public:

    SecurityEngine(const ExperimentConfig& config,
                   SimulationContext& context)
        : m_config(config),
          m_context(context)
    {
    }

    void Initialize()
    {
        Logger::Info("Security Engine initialized.");
    }

private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;

};

}

#endif
