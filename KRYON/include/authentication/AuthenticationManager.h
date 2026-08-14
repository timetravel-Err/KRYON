#ifndef KRYON_AUTHENTICATION_MANAGER_H
#define KRYON_AUTHENTICATION_MANAGER_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : AuthenticationManager.h
 *
 * Description
 * -----------
 * Manages the authentication protocol used by the framework.
 *
 * Currently the framework uses DummyAuthenticationProtocol.
 * Future phases can replace it with real authentication
 * protocols without affecting higher framework layers.
 * ----------------------------------------------------------
 */

#include "../core/ExperimentConfig.h"
#include "../simulation/SimulationContext.h"
#include "../core/Logger.h"
#include <memory>
#include "AuthenticationRequest.h"
#include "AuthenticationResult.h"
#include "IAuthenticationProtocol.h"
#include "protocols/rap/RAPAuthenticationProtocol.h"
#include "SessionManager.h"

#include "packets/AuthRequestPacket.h"
#include "packets/AuthChallengePacket.h"
#include "packets/AuthResponsePacket.h"
#include "packets/AuthConfirmPacket.h"
#include <functional>

namespace kryon
{

class AuthenticationManager
{
public:

	using CompletionHandler =
    std::function<void(const std::string&, bool)>;

    AuthenticationManager(const ExperimentConfig& config,
                          SimulationContext& context)
        : m_config(config),
          m_context(context),
		  m_sessionManager(config, context)
    {
    }
	
	

void SetCompletionHandler(CompletionHandler handler)
{
    m_completionHandler = handler;
}

void Initialize()
{
    Logger::Info("Authentication Manager starting...");
	
	m_sessionManager.Initialize();

    switch (m_config.authenticationProtocol)
    {
    case AuthenticationProtocolType::REFERENCE:

        Logger::Info("Authentication protocol : RAP");

        m_protocol =
            std::make_unique<RAPAuthenticationProtocol>();

        break;

    default:

        Logger::Info(
            "Requested protocol not implemented. Using RAP.");

        m_protocol =
            std::make_unique<RAPAuthenticationProtocol>();

        break;
    }

    // --------------------------------------------------
    // Inject CryptoEngine AFTER protocol is created
    // --------------------------------------------------

    if (m_crypto != nullptr)
    {
        auto* rap =
            dynamic_cast<RAPAuthenticationProtocol*>(m_protocol.get());

        if (rap)
        {
            Logger::Info("Injecting CryptoEngine into RAP");

            rap->SetCryptoEngine(m_crypto);
        }
    }

    m_protocol->Initialize();

    Logger::Info("Authentication Manager initialized.");
}

void ProcessAuthenticationRequest(
    const AuthRequestPacket& packet)
{
    Logger::Info(
        "[AuthenticationManager] Processing packet : " +
        packet.requestId);

    AuthenticationRequest request;

    request.requestId = packet.requestId;
    request.sourceNodeId = packet.sourceNode;
    request.destinationNodeId = packet.destinationNode;

    Authenticate(request);
}

void ProcessAuthenticationChallenge(
    const AuthChallengePacket& packet)
{
    Logger::Info(
        "[AuthenticationManager] Received AUTH_CHALLENGE : " +
        packet.requestId);
}

void ProcessAuthenticationResponse(
    const AuthResponsePacket& packet)
{
    Logger::Info(
        "[AuthenticationManager] Received AUTH_RESPONSE : " +
        packet.requestId);
}

void ProcessAuthenticationConfirm(
    const AuthConfirmPacket& packet)
{
    Logger::Info(
        "[AuthenticationManager] Received AUTH_CONFIRM : " +
        packet.requestId);
}

AuthenticationResult Authenticate(
    const AuthenticationRequest& request)
{
    // --------------------------------------------------
    // Record authentication attempt
    // --------------------------------------------------

    m_context.security.statistics.authenticationAttempts++;

    double currentTime =
        ns3::Simulator::Now().GetSeconds();

    /*
     * Remove expired sessions before searching.
     */
    m_sessionManager.RemoveExpiredSessions(
        currentTime);

    /*
     * --------------------------------------------------
     * Check whether an authenticated session already exists
     * --------------------------------------------------
     */
    Session* existingSession =
        m_sessionManager.FindSession(
            request.sourceNodeId,
            request.destinationNodeId,
            currentTime);

    if (existingSession)
    {
        Logger::Info(
            "Existing session found : " +
            existingSession->sessionId);

        AuthenticationResult result;

        result.requestId =
            request.requestId;

        result.protocolName =
            "SESSION-RESUMPTION";

        result.method =
            request.method;

        result.status =
            AuthenticationStatus::SUCCESS;

        result.authenticated =
            true;

        result.sessionId =
            existingSession->sessionId;

        result.sessionKey =
            existingSession->sessionKey;

        result.sessionLifetime =
            existingSession->expirationTime -
            existingSession->creationTime;

        result.messagesExchanged = 0;
        result.bytesExchanged = 0;
        result.authenticationTimeMs = 0.0;

        result.reason =
            "Existing authenticated session reused.";

        // --------------------------------------------------
        // Record successful session resumption
        // --------------------------------------------------

        m_context.security.statistics.authenticationSuccesses++;

        // --------------------------------------------------
        // Notify scheduler
        // --------------------------------------------------

        if (m_completionHandler)
        {
            m_completionHandler(
                request.requestId,
                result.authenticated);
        }

        return result;
    }

    /*
     * --------------------------------------------------
     * Perform full authentication protocol
     * --------------------------------------------------
     */
    if (m_protocol)
    {
       AuthenticationResult result =
    m_protocol->Authenticate(request);

		// --------------------------------------------------
		// Store authentication metrics for experiment export
		// --------------------------------------------------

		m_context.metrics.authenticationProtocol =
			result.protocolName;

		m_context.metrics.authenticationTimeMs =
			result.authenticationTimeMs;

		m_context.metrics.authenticationMessages =
			result.messagesExchanged;

		m_context.metrics.authenticationBytes =
			result.bytesExchanged;

		m_context.metrics.authenticationSuccess =
			result.authenticated;

        /*
         * --------------------------------------------------
         * Successful authentication
         * --------------------------------------------------
         */
        if (result.authenticated)
        {
            // --------------------------------------------------
            // Record successful authentication
            // --------------------------------------------------

            m_context.security.statistics.authenticationSuccesses++;

            Session session;

            session.sessionId =
                "SESSION-" + request.requestId;

            session.droneId =
                request.sourceNodeId;

            session.vehicleId =
                request.destinationNodeId;

            session.sessionKey =
                result.sessionKey;

            session.creationTime =
                ns3::Simulator::Now().GetSeconds();

            session.expirationTime =
                session.creationTime +
                result.sessionLifetime;

            session.active = true;

            m_sessionManager.CreateSession(session);

            result.sessionId =
                session.sessionId;

            Logger::Info(
                "AuthenticationManager stored session : " +
                session.sessionId);
        }
        else
        {
            // --------------------------------------------------
            // Record failed authentication
            // --------------------------------------------------

            m_context.security.statistics.authenticationFailures++;
        }

        /*
         * --------------------------------------------------
         * Notify scheduler that authentication is complete
         * --------------------------------------------------
         */

        if (m_completionHandler)
        {
            m_completionHandler(
                request.requestId,
                result.authenticated);
        }

        return result;
    }

    /*
     * --------------------------------------------------
     * Authentication protocol not initialized
     * --------------------------------------------------
     */

    AuthenticationResult result;

    result.requestId =
        request.requestId;

    result.method =
        request.method;

    result.status =
        AuthenticationStatus::FAILED;

    result.authenticated =
        false;

    result.reason =
        "Authentication protocol not initialized.";

    m_context.security.statistics.authenticationFailures++;

    /*
     * Notify scheduler even when authentication
     * could not be performed.
     */
    if (m_completionHandler)
    {
        m_completionHandler(
            request.requestId,
            false);
    }

    return result;
}


void Finalize()
{
	m_sessionManager.Finalize();
	
    if (m_protocol)
    {
        m_protocol->Finalize();
    }

    Logger::Info("Authentication Manager finalized.");
}

void SetCryptoEngine(CryptoEngine* crypto)
{
    Logger::Info("AuthenticationManager received CryptoEngine");

    m_crypto = crypto;

    // If protocol already exists, inject immediately

    if (m_protocol)
    {
        auto* rap =
            dynamic_cast<RAPAuthenticationProtocol*>(m_protocol.get());

        if (rap)
        {
            Logger::Info("Passing CryptoEngine to RAP");

            rap->SetCryptoEngine(crypto);
        }
    }
}

	SessionManager& GetSessionManager()
{
    return m_sessionManager;
}

private:

    const ExperimentConfig& m_config;
	
	CryptoEngine* m_crypto = nullptr;

    SimulationContext& m_context;

   std::unique_ptr<IAuthenticationProtocol> m_protocol;

	SessionManager m_sessionManager;

	CompletionHandler m_completionHandler;
	
	
};


}

#endif