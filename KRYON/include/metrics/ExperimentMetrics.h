#ifndef KRYON_EXPERIMENT_METRICS_H
#define KRYON_EXPERIMENT_METRICS_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : ExperimentMetrics.h
 *
 * Description
 * -----------
 * Stores all metrics generated during a single simulation
 * experiment.
 *
 * This structure serves as the central repository for
 * network, authentication, security, trust, blockchain,
 * and future protocol evaluation metrics.
 * ----------------------------------------------------------
 */

#include <string>
#include <cstdint>

namespace kryon
{

struct ExperimentMetrics
{
    /* ---------- Network Metrics ---------- */

    double throughput = 0.0;
    double delay = 0.0;
    double jitter = 0.0;
    double pdr = 0.0;

    /* ---------- Authentication Metrics ---------- */

    std::string authenticationProtocol;

    double authenticationTimeMs = 0.0;

    uint32_t authenticationMessages = 0;

    uint32_t authenticationBytes = 0;

    bool authenticationSuccess = false;

    /* ---------- Security Metrics ---------- */

    uint64_t authenticationAttempts = 0;

    uint64_t authenticationSuccesses = 0;

    uint64_t authenticationFailures = 0;
};

} // namespace kryon

#endif
