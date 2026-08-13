#ifndef KRYON_AUTH_CONFIRM_PACKET_H
#define KRYON_AUTH_CONFIRM_PACKET_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : AuthConfirmPacket.h
 *
 * Description
 * -----------
 * Represents the fourth RAP authentication message.
 *
 * Flow:
 *
 *   Vehicle -> Drone
 *
 * The confirmation informs the Drone whether mutual
 * authentication and session establishment succeeded.
 * ----------------------------------------------------------
 */

#include "AuthenticationPacket.h"

#include <string>
#include <vector>
#include <cstdint>

namespace kryon
{

class AuthConfirmPacket : public AuthenticationPacket
{
public:

    AuthConfirmPacket()
    {
        type = AuthenticationPacketType::AUTH_CONFIRM;
    }

    /*
     * Final authentication decision.
     */
    bool authenticationSuccessful = false;

    /*
     * Human-readable status message.
     */
    std::string message;

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

            sizeof(authenticationSuccessful) +

            sizeof(uint32_t) +
            message.size() +

            sizeof(uint32_t) +
            integrityHash.size();
    }
};

}

#endif