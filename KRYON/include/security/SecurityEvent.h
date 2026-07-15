#ifndef KRYON_SECURITY_EVENT_H
#define KRYON_SECURITY_EVENT_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : SecurityEvent.h
 *
 * Description
 * -----------
 * Represents a single security event generated during
 * simulation execution.
 *
 * Future examples:
 *
 * - Authentication started
 * - Authentication successful
 * - Authentication failed
 * - Session created
 * - Session expired
 * - Session terminated
 * - Key generated
 * - Trust updated
 * ----------------------------------------------------------
 */

#include <string>
#include "SecurityTypes.h"

namespace kryon
{

struct SecurityEvent
{
    /* ---------- Event Type ---------- */

    SecurityEventType type = SecurityEventType::AUTH_START;

    /* ---------- Simulation Time ---------- */

    double timestamp = 0.0;

    /* ---------- Source Node ---------- */

    uint32_t sourceNode = 0;

    /* ---------- Destination Node ---------- */

    uint32_t destinationNode = 0;

    /* ---------- Description ---------- */

    std::string description;

    /* ---------- Success ---------- */

    bool success = false;
};

} // namespace kryon

#endif
