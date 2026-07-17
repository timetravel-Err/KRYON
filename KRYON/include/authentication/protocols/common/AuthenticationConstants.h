#ifndef KRYON_AUTHENTICATION_CONSTANTS_H
#define KRYON_AUTHENTICATION_CONSTANTS_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : AuthenticationConstants.h
 *
 * Description
 * -----------
 * Common authentication constants shared across all
 * authentication protocols.
 *
 * These values are protocol-independent and may be reused
 * by Dummy, 2PQS-IoAV, TC2PA, SLAP, DID, Blockchain-based,
 * and future authentication schemes.
 * ----------------------------------------------------------
 */

#include <cstdint>

namespace kryon
{

namespace AuthenticationConstants
{

/* ---------- General ---------- */

constexpr uint32_t DEFAULT_TIMEOUT_MS = 5000;

constexpr uint32_t DEFAULT_MAX_RETRIES = 3;

/* ---------- Session ---------- */

constexpr uint32_t DEFAULT_SESSION_KEY_SIZE = 32;

constexpr uint32_t DEFAULT_NONCE_SIZE = 32;

/* ---------- Protocol ---------- */

constexpr uint32_t DEFAULT_REQUEST_ID_LENGTH = 16;

}

} // namespace kryon

#endif
