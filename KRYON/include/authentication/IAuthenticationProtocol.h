#ifndef KRYON_I_AUTHENTICATION_PROTOCOL_H
#define KRYON_I_AUTHENTICATION_PROTOCOL_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : IAuthenticationProtocol.h
 *
 * Description
 * -----------
 * Abstract interface implemented by all authentication
 * protocols supported by KRYON.
 *
 * The framework interacts only through this interface,
 * allowing different authentication schemes to be plugged
 * into the simulator without modifying framework code.
 *
 * Future Implementations
 * ----------------------
 * • DummyAuthenticationProtocol
 * • 2PQS-IoAV
 * • TC2PA
 * • SLAP
 * • DID Authentication
 * • VC Authentication
 * • PUF-based Authentication
 * • Blockchain Authentication
 * ----------------------------------------------------------
 */

#include <string>

#include "AuthenticationRequest.h"
#include "AuthenticationResult.h"
#include "../crypto/CryptoEngine.h"
namespace kryon
{

class IAuthenticationProtocol
{
public:

    virtual ~IAuthenticationProtocol() = default;

    /**
     * Initialize protocol resources.
     */
    virtual void Initialize() = 0;
	//virtual void Initialize( CryptoEngine& crypto) = 0;
    /**
     * Execute one authentication operation.
     */
    virtual AuthenticationResult Authenticate(
        const AuthenticationRequest& request) = 0;

    /**
     * Release protocol resources.
     */
    virtual void Finalize() = 0;

    /**
     * Return protocol name.
     */
    virtual std::string GetProtocolName() const = 0;
};

}

#endif
