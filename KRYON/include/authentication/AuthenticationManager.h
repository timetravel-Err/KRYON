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

#include "AuthenticationRequest.h"
#include "AuthenticationResult.h"

#include "protocols/rap/RAPAuthenticationProtocol.h"
namespace kryon
{

class AuthenticationManager
{
public:

    AuthenticationManager(const ExperimentConfig& config,
                          SimulationContext& context)
        : m_config(config),
          m_context(context)
    {
    }

   void Initialize()
{
    Logger::Info("Authentication Manager starting...");

    Logger::Info(
        "Authentication protocol: REFERENCE");

    m_protocol.Initialize();

    Logger::Info(
        "Authentication Manager initialized.");
}
    AuthenticationResult Authenticate(
        const AuthenticationRequest& request)
    {
        return m_protocol.Authenticate(request);
    }

    void Finalize()
    {
        m_protocol.Finalize();

        Logger::Info("Authentication Manager finalized.");
    }

private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;

    RAPAuthenticationProtocol m_protocol;
};

}

#endif