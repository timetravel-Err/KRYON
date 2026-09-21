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
			* Scheduler-level authentication stages.
			 *
			 * The scheduler tracks the authentication transaction,
			 * while RAPAuthenticationProtocol owns the internal
			 * protocol message sequence.
			 *
			 * 0 -> Authentication not started
			 * 1 -> Authentication request transmitted
			 * 2 -> Authentication completed
			 */
			uint32_t totalSteps = 2;
	
		/*
	 * Measured authentication latency.
	 *
	 * This is the elapsed simulation time from the moment
	 * authentication starts to the moment the completion
	 * callback is received.
	 */
	double authenticationLatency = 0.0;
	
	uint32_t currentStep = 0;

    bool completed = false;

    bool success = false;
};

}

#endif
