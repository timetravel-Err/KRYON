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
#include "packets/AuthRequestPacket.h"
#include "packets/AuthChallengePacket.h"
#include "packets/AuthResponsePacket.h"
#include "packets/AuthConfirmPacket.h"

namespace kryon
{

class IAuthenticationProtocol
{
public:

    virtual ~IAuthenticationProtocol() = default;

    virtual void Initialize() = 0;

    virtual AuthenticationResult Authenticate(
        const AuthenticationRequest& request) = 0;

    virtual AuthChallengePacket ProcessRequest(
        const AuthRequestPacket& packet) = 0;

    virtual AuthResponsePacket ProcessChallenge(
        const AuthChallengePacket& packet) = 0;

    virtual AuthConfirmPacket ProcessResponse(
        const AuthResponsePacket& packet) = 0;

    virtual void Finalize() = 0;

    virtual std::string GetProtocolName() const = 0;
};

}

#endif
