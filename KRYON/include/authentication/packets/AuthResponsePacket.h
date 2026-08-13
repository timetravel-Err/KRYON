#ifndef KRYON_AUTH_RESPONSE_PACKET_H
#define KRYON_AUTH_RESPONSE_PACKET_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : AuthResponsePacket.h
 *
 * Description
 * -----------
 * Represents the third RAP authentication message.
 *
 * Flow:
 *
 *   Drone -> Vehicle
 *
 * The response contains:
 *   - Challenge received from Vehicle
 *   - SHA-256 authentication proof
 *   - Drone public key
 *   - Drone digital signature
 *   - Integrity hash
 * ----------------------------------------------------------
 */

#include "AuthenticationPacket.h"

#include <vector>
#include <cstdint>

namespace kryon
{

class AuthResponsePacket : public AuthenticationPacket
{
public:

    AuthResponsePacket()
    {
        type = AuthenticationPacketType::AUTH_RESPONSE;
    }

    /*
     * Echo of the challenge received from the Vehicle.
     */
    std::vector<uint8_t> challenge;

    /*
     * Authentication proof.
     *
     * Current RAP implementation:
     * SHA-256(challenge)
     */
    std::vector<uint8_t> proof;

    /*
     * Drone public key.
     */
    std::vector<uint8_t> senderPublicKey;

    /*
     * Drone digital signature.
     */
    std::vector<uint8_t> signature;

    /*
     * Optional integrity protection.
     */
    std::vector<uint8_t> integrityHash;

    /*
     * Serialized packet size.
     */
    uint32_t GetPacketSize() const override
    {
        return
            sizeof(uint8_t) +

            sizeof(uint32_t) +
            requestId.size() +

            sizeof(sourceNode) +

            sizeof(destinationNode) +

            sizeof(timestamp) +

            sizeof(uint32_t) +
            challenge.size() +

            sizeof(uint32_t) +
            proof.size() +

            sizeof(uint32_t) +
            senderPublicKey.size() +

            sizeof(uint32_t) +
            signature.size() +

            sizeof(uint32_t) +
            integrityHash.size();
    }
};

}

#endif