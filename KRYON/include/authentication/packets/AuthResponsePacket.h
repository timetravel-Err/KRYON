#ifndef KRYON_AUTH_RESPONSE_PACKET_H
#define KRYON_AUTH_RESPONSE_PACKET_H

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

    std::vector<uint8_t> authenticationData;

    std::vector<uint8_t> integrityHash;

    uint32_t GetPacketSize() const override
    {
        return sizeof(type)
             + requestId.size()
             + sizeof(sourceNode)
             + sizeof(destinationNode)
             + sizeof(timestamp)
             + authenticationData.size()
             + integrityHash.size();
    }
};

}

#endif
