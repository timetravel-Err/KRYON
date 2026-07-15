#ifndef KRYON_SECURITY_TYPES_H
#define KRYON_SECURITY_TYPES_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : SecurityTypes.h
 *
 * Description
 * -----------
 * Common security enumerations used throughout
 * the KRYON security framework.
 *
 * These types are shared by:
 *
 * - SecurityEngine
 * - SecuritySession
 * - SecurityEvent
 * - SecurityStatistics
 * - Future authentication protocols
 * ----------------------------------------------------------
 */

namespace kryon
{

/* ---------- Authentication ---------- */

enum class AuthenticationState
{
    NOT_STARTED,
    IN_PROGRESS,
    SUCCESS,
    FAILED
};

/* ---------- Session ---------- */

enum class SessionState
{
    CREATED,
    ACTIVE,
    EXPIRED,
    TERMINATED
};

/* ---------- Trust ---------- */

enum class TrustState
{
    UNKNOWN,
    TRUSTED,
    SUSPICIOUS,
    MALICIOUS
};

/* ---------- Security Events ---------- */

enum class SecurityEventType
{
    AUTH_START,
    AUTH_SUCCESS,
    AUTH_FAILURE,
    SESSION_CREATED,
    SESSION_TERMINATED,
    KEY_GENERATED,
    TRUST_UPDATED
};

} // namespace kryon

#endif
