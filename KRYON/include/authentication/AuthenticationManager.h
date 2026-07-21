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

m_protocol->Initialize();

Logger::Info("Authentication Manager initialized.");
}
    AuthenticationResult Authenticate(
        const AuthenticationRequest& request)
    {
         if (m_protocol)
    {
        return m_protocol->Authenticate(request);
    }

    AuthenticationResult result;

    result.requestId = request.requestId;
    result.method = request.method;
    result.status = AuthenticationStatus::FAILED;
    result.authenticated = false;
    result.reason = "Authentication protocol not initialized.";

    return result;
    }

    void Finalize()
    {
       if (m_protocol)
    {
        m_protocol->Finalize();
    }

    Logger::Info("Authentication Manager finalized.");
    }

private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;

    std::unique_ptr<IAuthenticationProtocol> m_protocol;
};

}

#endif