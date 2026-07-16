#ifndef KRYON_AUTHENTICATION_CONTEXT_H
#define KRYON_AUTHENTICATION_CONTEXT_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : AuthenticationContext.h
 *
 * Description
 * -----------
 * Maintains runtime state for the Authentication
 * subsystem.
 *
 * Stores all authentication requests, responses,
 * and final authentication results generated during
 * a simulation.
 * ----------------------------------------------------------
 */

#include <vector>

#include "AuthenticationRequest.h"
#include "AuthenticationResponse.h"
#include "AuthenticationResult.h"

namespace kryon
{

struct AuthenticationContext
{
    std::vector<AuthenticationRequest> requests;

    std::vector<AuthenticationResponse> responses;

    std::vector<AuthenticationResult> results;
};

}

#endif
