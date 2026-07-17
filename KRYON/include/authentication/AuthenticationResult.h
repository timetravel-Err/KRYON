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

namespace kryon
{

struct AuthenticationResult
{
    std::string requestId;

    AuthenticationMethod method =
        AuthenticationMethod::NONE;

    AuthenticationStatus status =
        AuthenticationStatus::NOT_STARTED;

   bool authenticated = false;

double authenticationTimeMs = 0.0;

std::string reason;
};

}

#endif
