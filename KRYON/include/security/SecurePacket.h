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
	 * AES-256-GCM nonce / IV.
	 *
	 * GCM uses a 12-byte nonce for each encryption
	 * operation. The nonce is not secret and must be
	 * transmitted with the ciphertext.
	 */
	ByteArray nonce;

	/*
	 * AES-256-GCM authentication tag.
	 *
	 * This is carried in the existing MAC field for
	 * compatibility with the SecurePacket abstraction.
	 */
	ByteArray mac;

    /*
     * Indicates encryption status.
     */
    bool encrypted = false;
};

}

#endif
