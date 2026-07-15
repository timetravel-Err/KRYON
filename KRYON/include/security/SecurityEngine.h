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
#include "SecuritySession.h"
#include "SecurityEvent.h"
#include "SecurityStatistics.h"
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
    m_context.security.authenticationEnabled = true;

    Logger::Info("Security Engine initialized.");
}

void StartSession(SecuritySession session)
{
    m_context.security.sessions.push_back(session);

    m_context.security.statistics.sessionsCreated++;
}

void RecordEvent(const SecurityEvent& event)
{
    m_context.security.events.push_back(event);

    switch (event.type)
    {
        case SecurityEventType::AUTH_START:
            m_context.security.statistics.authenticationAttempts++;
            break;

        case SecurityEventType::AUTH_SUCCESS:
            m_context.security.statistics.authenticationSuccesses++;
            break;

        case SecurityEventType::AUTH_FAILURE:
            m_context.security.statistics.authenticationFailures++;
            break;

        case SecurityEventType::KEY_GENERATED:
            m_context.security.statistics.keysGenerated++;
            break;

        case SecurityEventType::TRUST_UPDATED:
            m_context.security.statistics.trustUpdates++;
            break;

        default:
            break;
    }
}

void Finalize()
{
    Logger::Info("Security Engine finalized.");
}
private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;

};

}

#endif
