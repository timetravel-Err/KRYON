#ifndef KRYON_AUTH_REQUEST_PACKET_H
#define KRYON_AUTH_REQUEST_PACKET_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : AuthRequestPacket.h
 *
 * Description
 * -----------
 * Represents the first RAP authentication packet.
 *
 * Flow:
 *
 *   Drone -> Vehicle
 * ----------------------------------------------------------
 */

#include "AuthenticationPacket.h"

#include <vector>
#include <cstdint>

namespace kryon
{

class AuthRequestPacket : public AuthenticationPacket
{
public:

    AuthRequestPacket()
    {
        type = AuthenticationPacketType::AUTH_REQUEST;
    }

    /*
     * Freshness nonce.
     */
    std::vector<uint8_t> nonce;

    /*
     * Protocol-specific authentication data.
     *
     * Examples:
     *   RAP       -> authentication information
     *   SLAP      -> PUF response
     *   2PQS      -> Kyber public key
     *   DID       -> Verifiable Credential
     */
    std::vector<uint8_t> authenticationData;

    /*
     * Integrity hash.
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
            nonce.size() +

            sizeof(uint32_t) +
            authenticationData.size() +

            sizeof(uint32_t) +
            integrityHash.size();
    }
};

}

#endif