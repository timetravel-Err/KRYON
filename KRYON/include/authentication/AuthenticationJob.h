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
	
	/*
	 * Current RAP protocol step
	 *
	 * 0 -> Not started
	 * 1 -> Message 1
	 * 2 -> Message 2
	 * 3 -> Message 3
	 * 4 -> Key Agreement
	 * 5 -> Session Established
	 */
	 
	/*
	 * Total RAP protocol steps
	 */
	uint32_t totalSteps = 5; 
	
	uint32_t currentStep = 0;

    bool completed = false;

    bool success = false;
};

}

#endif
