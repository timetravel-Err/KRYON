#ifndef KRYON_AUTHENTICATION_JOB_H
#define KRYON_AUTHENTICATION_JOB_H

#include "AuthenticationRequest.h"
#include "AuthenticationState.h"

namespace kryon
{

struct AuthenticationJob
{
    AuthenticationRequest request;

    AuthenticationState state =
        AuthenticationState::IDLE;

    double startTime = 0.0;

    double nextEventTime = 0.0;

    bool completed = false;

    bool success = false;
};

}

#endif
