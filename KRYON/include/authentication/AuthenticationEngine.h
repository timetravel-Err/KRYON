#ifndef KRYON_AUTHENTICATION_ENGINE_H
#define KRYON_AUTHENTICATION_ENGINE_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : AuthenticationEngine.h
 *
 * Description
 * -----------
 * Coordinates authentication operations within KRYON.
 *
 * This engine manages the authentication lifecycle while
 * remaining independent of any specific authentication
 * protocol or cryptographic implementation.
 *
 * Future protocol implementations (ECC, PUF, DID,
 * Blockchain, PQC, etc.) should integrate through this
 * engine rather than modifying framework components.
 * ----------------------------------------------------------
 */

#include "../core/ExperimentConfig.h"
#include "../simulation/SimulationContext.h"
#include "../core/Logger.h"

#include "AuthenticationContext.h"
#include "AuthenticationRequest.h"
#include "AuthenticationResponse.h"
#include "AuthenticationResult.h"
#include "AuthenticationTypes.h"
#include "AuthenticationManager.h"
#include <chrono>
namespace kryon
{

class AuthenticationEngine
{
public:

  AuthenticationEngine(const ExperimentConfig& config,
                     SimulationContext& context)
    : m_config(config),
      m_context(context),
      m_manager(config, context)
{
}
void SetCryptoEngine(CryptoEngine* crypto)
{
   Logger::Info("AuthenticationEngine received CryptoEngine");
   m_manager.SetCryptoEngine(crypto);
}
    void Initialize()
    {
		m_manager.Initialize();

        Logger::Info("Authentication Engine initialized.");
    }

    void StartAuthentication(const AuthenticationRequest& request)
    {
        m_context.security.authentication.requests.push_back(request);

        Logger::Info("Authentication request created.");
    }

   void ProcessAuthentication()
	{
    if (m_context.security.authentication.requests.empty())
    {
        return;
    }

    AuthenticationRequest request =
        m_context.security.authentication.requests.back();
		
	auto start =
    std::chrono::high_resolution_clock::now();	

    AuthenticationResult result =
        m_manager.Authenticate(request);
	
	auto end =
    std::chrono::high_resolution_clock::now();

	result.authenticationTimeMs =
    std::chrono::duration<double, std::milli>(end - start).count();

    m_context.security.authentication.results.push_back(result);
	
	/* ---------- Store experiment metrics ---------- */

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

    Logger::Info("========== Authentication Summary ==========");

Logger::Info(
    "Protocol            : " +
    result.protocolName);

Logger::Info(
    "Status              : " +
    std::string(result.authenticated ? "SUCCESS" : "FAILED"));

Logger::Info(
    "Messages Exchanged  : " +
    std::to_string(result.messagesExchanged));

Logger::Info(
    "Bytes Exchanged     : " +
    std::to_string(result.bytesExchanged));

Logger::Info(
    "Authentication Time : " +
    std::to_string(result.authenticationTimeMs) +
    " ms");

Logger::Info(
    "Reason              : " +
    result.reason);

Logger::Info("============================================");
	
	
	}
	
    void CompleteAuthentication(const AuthenticationResult& result)
    {
        m_context.security.authentication.results.push_back(result);

        Logger::Info("Authentication completed.");
    }

	void Finalize()
	{
    m_manager.Finalize();

    Logger::Info("Authentication Engine finalized.");
	}
	
	AuthenticationManager& GetAuthenticationManager()
	{
		return m_manager;
	}

private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;
	
	AuthenticationManager m_manager;
};

}

#endif
