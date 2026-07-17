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
#include "../crypto/CryptoEngine.h"

#include "../authentication/AuthenticationEngine.h"
namespace kryon
{

class SecurityEngine
{
public:

    SecurityEngine(const ExperimentConfig& config,
                   SimulationContext& context)
        : m_config(config),
          m_context(context),
		  m_crypto(config, context),
		  m_authentication(config, context)
    {
    }

    void Initialize()
{

    m_crypto.Initialize();

    //m_context.security.authenticationEnabled = true;
	m_authentication.Initialize();

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

void ExecuteAuthentication()
{
    AuthenticationRequest request;

   request.requestId = "REQ-0001";

	auto drone = m_context.drones.Get(0);
	auto av = m_context.avs.Get(0);

	request.sourceNodeId = drone->GetId();
	request.destinationNodeId = av->GetId();

	request.method = AuthenticationMethod::NONE;
	request.requiresMutualAuthentication = true;
	request.timestamp = ns3::Simulator::Now().GetSeconds();

    m_authentication.StartAuthentication(request);
    m_authentication.ProcessAuthentication();
    Logger::Info(
    "Authenticated Drone " +
    std::to_string(request.sourceNodeId) +
    " with Vehicle " +
    std::to_string(request.destinationNodeId));
    Logger::Info("Authentication workflow executed.");
}

void Finalize()
{
    m_authentication.Finalize();
	m_crypto.Finalize();

    Logger::Info("Security Engine finalized.");
}
private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;
	
	CryptoEngine m_crypto;
	AuthenticationEngine m_authentication;
};

}

#endif
