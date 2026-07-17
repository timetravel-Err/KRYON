#ifndef KRYON_DUMMY_AUTHENTICATION_PROTOCOL_H
#define KRYON_DUMMY_AUTHENTICATION_PROTOCOL_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : DummyAuthenticationProtocol.h
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

class DummyAuthenticationProtocol : public IAuthenticationProtocol
{
public:

    DummyAuthenticationProtocol() = default;

    ~DummyAuthenticationProtocol() override = default;

    void Initialize() override
    {
        Logger::Info("Dummy Authentication Protocol initialized.");
    }

    AuthenticationResult Authenticate(
        const AuthenticationRequest& request) override
    {
        (void)request;

        AuthenticationResult result;

        result.authenticated = true;

        Logger::Info("Dummy authentication succeeded.");

        return result;
    }

    void Finalize() override
    {
        Logger::Info("Dummy Authentication Protocol finalized.");
    }

    std::string GetProtocolName() const override
    {
        return "Dummy Authentication";
    }
};

}

#endif
