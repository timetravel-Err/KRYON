#ifndef KRYON_AUTHENTICATION_RESULT_H
#define KRYON_AUTHENTICATION_RESULT_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : AuthenticationResult.h
 *
 * Description
 * -----------
 * Represents the final outcome of an authentication
 * operation as recorded by the framework.
 *
 * Unlike AuthenticationResponse, this structure captures
 * the framework's final decision together with execution
 * statistics.
 * ----------------------------------------------------------
 */

#include <string>

#include "AuthenticationTypes.h"
#include "../crypto/CryptoTypes.h"

namespace kryon
{

struct AuthenticationResult
{
    std::string requestId;

    std::string protocolName;

    AuthenticationMethod method =
        AuthenticationMethod::NONE;

    AuthenticationStatus status =
        AuthenticationStatus::NOT_STARTED;

    bool authenticated = false;
	
	/*
 * Session established after successful authentication.
 */
	/*
 * Established session identifier.
 */
	std::string sessionId;

/*
 * Session key negotiated during authentication.
 */
	SessionKey sessionKey;

/*
 * Session lifetime (seconds).
 */
	double sessionLifetime = 0.0;

    uint32_t messagesExchanged = 0;

    uint32_t bytesExchanged = 0;

    double authenticationTimeMs = 0.0;

    std::string reason;
};

}

#endif
