#ifndef KRYON_AUTH_CONFIRM_PACKET_H
#define KRYON_AUTH_CONFIRM_PACKET_H

#include "AuthenticationPacket.h"
#include <string>

namespace kryon
{

class AuthConfirmPacket : public AuthenticationPacket
{
public:

    AuthConfirmPacket()
    {
        type = AuthenticationPacketType::AUTH_CONFIRM;
    }

    bool authenticationSuccessful = false;

    std::string message;

    uint32_t GetPacketSize() const override
    {
        return sizeof(type)
             + requestId.size()
             + sizeof(sourceNode)
             + sizeof(destinationNode)
             + sizeof(timestamp)
             + sizeof(authenticationSuccessful)
             + message.size();
    }
};

}

#endif
