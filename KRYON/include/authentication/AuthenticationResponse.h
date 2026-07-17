#ifndef KRYON_AUTHENTICATION_RESPONSE_H
#define KRYON_AUTHENTICATION_RESPONSE_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : AuthenticationResponse.h
 *
 * Description
 * -----------
 * Represents the response returned after processing an
 * authentication request.
 *
 * This structure is protocol-independent and is intended
 * to support future authentication mechanisms.
 * ----------------------------------------------------------
 */

#include <string>

#include "AuthenticationTypes.h"

namespace kryon
{

struct AuthenticationResponse
{
    std::string requestId;

    uint32_t responderNodeId = 0;

    uint64_t challenge = 0;

    std::string proof;

    double timestamp = 0.0;
};
}

#endif
