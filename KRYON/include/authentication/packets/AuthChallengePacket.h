#ifndef KRYON_AUTH_CHALLENGE_PACKET_H
#define KRYON_AUTH_CHALLENGE_PACKET_H

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

    std::vector<uint8_t> challenge;

    std::vector<uint8_t> integrityHash;

    uint32_t GetPacketSize() const override
    {
        return sizeof(type)
             + requestId.size()
             + sizeof(sourceNode)
             + sizeof(destinationNode)
             + sizeof(timestamp)
             + challenge.size()
             + integrityHash.size();
    }
};

}

#endif
