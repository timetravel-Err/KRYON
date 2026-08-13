#ifndef KRYON_AUTH_CHALLENGE_PACKET_H
#define KRYON_AUTH_CHALLENGE_PACKET_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : AuthChallengePacket.h
 *
 * Description
 * -----------
 * Represents the second RAP authentication message.
 *
 * Flow:
 *
 *   Vehicle -> Drone
 *
 * The challenge contains:
 *   - Random challenge/nonce
 *   - Vehicle public key
 *   - Vehicle digital signature
 *   - Integrity hash
 * ----------------------------------------------------------
 */

#include "AuthenticationPacket.h"

#include <vector>
#include <cstdint>

namespace kryon
{

class AuthChallengePacket : public AuthenticationPacket
{
public:

    AuthChallengePacket()
    {
        type = AuthenticationPacketType::AUTH_CHALLENGE;
    }

    /*
     * Random authentication challenge.
     */
    std::vector<uint8_t> challenge;

    /*
     * Sender's public key.
     *
     * For RAP this is the Vehicle public key.
     */
    std::vector<uint8_t> senderPublicKey;

    /*
     * Digital signature over the challenge.
     */
    std::vector<uint8_t> signature;

    /*
     * Optional integrity protection.
     */
    std::vector<uint8_t> integrityHash;

    /*
     * Size of the serialized packet payload.
     *
     * PacketWriter stores:
     *
     * type
     * requestId length + requestId
     * sourceNode
     * destinationNode
     * timestamp
     * challenge length + challenge
     * public-key length + public key
     * signature length + signature
     * integrity-hash length + hash
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
            senderPublicKey.size() +

            sizeof(uint32_t) +
            signature.size() +

            sizeof(uint32_t) +
            integrityHash.size();
    }
};

}

#endif