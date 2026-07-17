#ifndef KRYON_RAP _AUTHENTICATION_PROTOCOL_H
#define KRYON_RAP _AUTHENTICATION_PROTOCOL_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : RAPAuthenticationProtocol.h
 *
 * Description
 * -----------
 * A simple authentication protocol used only for validating
 * the authentication framework architecture.
 *
 * This protocol performs no cryptographic operations and
 * always returns a successful authentication result.
 * ----------------------------------------------------------
 */

#include "../../IAuthenticationProtocol.h"
#include "../../AuthenticationRequest.h"
#include "../../AuthenticationResult.h"
#include "../../AuthenticationTypes.h"
#include "../../../core/Logger.h"

namespace kryon
{

class RAPAuthenticationProtocol : public IAuthenticationProtocol
{
public:

    RAPAuthenticationProtocol() = default;

    ~RAPAuthenticationProtocol() override = default;

    void Initialize() override
    {
        Logger::Info("RAP  Authentication Protocol initialized.");
    }

    AuthenticationResult Authenticate(
        const AuthenticationRequest& request) override
    {
        (void)request;

        AuthenticationResult result;

        result.authenticated = true;

        Logger::Info("RAP  authentication succeeded.");

        return result;
    }

    void Finalize() override
    {
        Logger::Info("RAP  Authentication Protocol finalized.");
    }

    std::string GetProtocolName() const override
    {
        return "RReference Authentication Protocol (RAP)";
    }
};

}

#endif
