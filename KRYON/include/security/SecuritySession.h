#ifndef KRYON_SECURITY_SESSION_H
#define KRYON_SECURITY_SESSION_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : SecuritySession.h
 *
 * Description
 * -----------
 * Represents one logical security session established
 * between two communicating nodes.
 *
 * The session stores the authentication state,
 * session state and negotiated session key.
 * ----------------------------------------------------------
 */

#include <string>

#include "SecurityTypes.h"

namespace kryon
{

struct SecuritySession
{
    /* ---------- Session ID ---------- */

    uint32_t sessionId = 0;

    /* ---------- Nodes ---------- */

    uint32_t sourceNode = 0;
    uint32_t destinationNode = 0;

    /* ---------- States ---------- */

    AuthenticationState authenticationState =
        AuthenticationState::NOT_STARTED;

    SessionState sessionState =
        SessionState::CREATED;

    /* ---------- Session Key ---------- */

    std::string sessionKey;

    /* ---------- Timing ---------- */

    double startTime = 0.0;
    double endTime = 0.0;

    /* ---------- Status ---------- */

    bool authenticated = false;
};

} // namespace kryon

#endif
