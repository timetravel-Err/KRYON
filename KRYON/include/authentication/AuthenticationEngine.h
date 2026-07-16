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

namespace kryon
{

class AuthenticationEngine
{
public:

    AuthenticationEngine(const ExperimentConfig& config,
                         SimulationContext& context)
        : m_config(config),
          m_context(context)
    {
    }

    void Initialize()
    {
        Logger::Info("Authentication Engine initialized.");
    }

    void StartAuthentication(const AuthenticationRequest& request)
    {
        m_context.security.authentication.requests.push_back(request);

        Logger::Info("Authentication request created.");
    }

    void ProcessAuthentication()
    {
        Logger::Info("Authentication processing started.");
    }

    void CompleteAuthentication(const AuthenticationResult& result)
    {
        m_context.security.authentication.results.push_back(result);

        Logger::Info("Authentication completed.");
    }

    void Finalize()
    {
        Logger::Info("Authentication Engine finalized.");
    }

private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;
};

}

#endif
