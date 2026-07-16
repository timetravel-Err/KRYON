#ifndef KRYON_AUTHENTICATION_REQUEST_H
#define KRYON_AUTHENTICATION_REQUEST_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : AuthenticationRequest.h
 *
 * Description
 * -----------
 * Represents an authentication request exchanged between
 * two entities within the framework.
 *
 * This structure is protocol-independent and can be used
 * by future authentication schemes.
 * ----------------------------------------------------------
 */

#include <string>

#include "AuthenticationTypes.h"

namespace kryon
{

struct AuthenticationRequest
{
    std::string requestId;

    uint32_t sourceNodeId = 0;

    uint32_t destinationNodeId = 0;

    AuthenticationMethod method =
        AuthenticationMethod::NONE;

    bool requiresMutualAuthentication = false;

    double timestamp = 0.0;
};

}

#endif
