#ifndef KRYON_SECURITY_STATISTICS_H
#define KRYON_SECURITY_STATISTICS_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : SecurityStatistics.h
 *
 * Description
 * -----------
 * Collects generic security statistics generated
 * during simulation.
 *
 * These statistics are protocol-independent and can
 * be updated by any authentication or security scheme.
 * ----------------------------------------------------------
 */

namespace kryon
{

struct SecurityStatistics
{
    /* ---------- Authentication ---------- */

    uint64_t authenticationAttempts = 0;
    uint64_t authenticationSuccesses = 0;
    uint64_t authenticationFailures = 0;

    /* ---------- Sessions ---------- */

    uint64_t sessionsCreated = 0;
    uint64_t sessionsExpired = 0;
    uint64_t sessionsTerminated = 0;

    /* ---------- Keys ---------- */

    uint64_t keysGenerated = 0;

    /* ---------- Trust ---------- */

    uint64_t trustUpdates = 0;

    /* ---------- Timing ---------- */

    double averageAuthenticationTime = 0.0;
};

} // namespace kryon

#endif
