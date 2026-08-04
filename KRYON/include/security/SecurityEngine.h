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
#include "SecureChannel.h"

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
	
	Logger::Info("SecurityEngine: Passing CryptoEngine");

	m_authentication.SetCryptoEngine(&m_crypto);

	m_authentication.Initialize();
	
	m_secureChannel.SetCryptoEngine(
    &m_crypto);

	m_secureChannel.Initialize();

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

void ExecuteAuthentication(AuthenticationRequest request)
{
    
	SecurityEvent startEvent;

	startEvent.type = SecurityEventType::AUTH_START;

	RecordEvent(startEvent);

	m_authentication.StartAuthentication(request);
    m_authentication.ProcessAuthentication();
	const auto& result = m_context.security.authentication.results.back();
	
	SecurityEvent resultEvent;

	resultEvent.type = result.authenticated ? SecurityEventType::AUTH_SUCCESS : SecurityEventType::AUTH_FAILURE;

	RecordEvent(resultEvent);
    Logger::Info(
    "Authenticated Drone " +
    std::to_string(request.sourceNodeId) +
    " with Vehicle " +
    std::to_string(request.destinationNodeId));
    Logger::Info("Authentication workflow executed.");
	
	/*
 * --------------------------------------------------
 * Demonstration of Secure Channel
 * --------------------------------------------------
 */

if (result.authenticated)
{
    Session* session =
        m_authentication
            .GetAuthenticationManager()
            .GetSessionManager()
            .FindSession(
                request.sourceNodeId,
                request.destinationNodeId,
                ns3::Simulator::Now().GetSeconds());
	Logger::Info(
    std::string("Session pointer = ") +
    (session ? "FOUND" : "NULL"));
    if (session)
    {
        ByteArray payload;

        std::string message =
            "Hello Secure KRYON";

        payload.data.assign(
            message.begin(),
            message.end());

        Logger::Info(
            "Encrypting application payload.");

        SecurePacket packet =
            m_secureChannel.Encrypt(
                payload,
                *session);

        ByteArray recovered =
            m_secureChannel.Decrypt(
                packet,
                *session);

        std::string recoveredMessage(
            recovered.data.begin(),
            recovered.data.end());

        Logger::Info(
            "Recovered Payload : " +
            recoveredMessage);
    }
}

}

void PrintSecurityStatistics()
{
    auto& stats = m_context.security.statistics;

    double successRate = 0.0;

    if (stats.authenticationAttempts > 0)
    {
        successRate =
            (100.0 * stats.authenticationSuccesses) /
            stats.authenticationAttempts;
    }

    Logger::Info("==========================================");
    Logger::Info("KRYON Security Statistics");
    Logger::Info("==========================================");

    Logger::Info(
        "Authentication Attempts : " +
        std::to_string(stats.authenticationAttempts));

    Logger::Info(
        "Authentication Success  : " +
        std::to_string(stats.authenticationSuccesses));

    Logger::Info(
        "Authentication Failure  : " +
        std::to_string(stats.authenticationFailures));

    Logger::Info(
        "Success Rate            : " +
        std::to_string(successRate) + " %");

    Logger::Info("==========================================");
}

	SecureChannel& GetSecureChannel()
	{
		return m_secureChannel;
	}

	void Finalize()
	{
		m_authentication.Finalize();
		m_crypto.Finalize();
		m_secureChannel.Finalize();
		Logger::Info("Security Engine finalized.");
	}
	
private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;
	
	CryptoEngine m_crypto;
	AuthenticationEngine m_authentication;
	SecureChannel m_secureChannel;
	
};

}

#endif
