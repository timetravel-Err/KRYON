#ifndef KRYON_SECURE_PACKET_H
#define KRYON_SECURE_PACKET_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : SecurePacket.h
 *
 * Description
 * -----------
 * Generic encrypted packet exchanged after successful
 * authentication.
 *
 * This structure is independent of the authentication
 * protocol (RAP, SLAP, 2PQS, DID, etc.).
 *
 * Responsibilities
 * ----------------
 * • Carry encrypted payload
 * • Carry authentication tag (MAC)
 * • Carry session identifier
 * • Carry replay protection fields
 * ----------------------------------------------------------
 */

#include <string>

#include "../crypto/CryptoTypes.h"

namespace kryon
{

struct SecurePacket
{
    /*
     * Session information
     */
    std::string sessionId;

    /*
     * Sequence number
     * Used to prevent replay attacks.
     */
    uint64_t sequenceNumber = 0;

    /*
     * Packet creation timestamp.
     */
    double timestamp = 0.0;

    /*
     * Encrypted application payload.
     */
    ByteArray ciphertext;

    /*
     * Authentication tag / MAC.
     */
    ByteArray mac;

    /*
     * Indicates encryption status.
     */
    bool encrypted = false;
};

}

#endif
