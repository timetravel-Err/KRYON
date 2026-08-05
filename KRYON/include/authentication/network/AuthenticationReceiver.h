#ifndef KRYON_AUTHENTICATION_RECEIVER_H
#define KRYON_AUTHENTICATION_RECEIVER_H

#include "../packets/AuthenticationPacket.h"
#include "../../core/Logger.h"

namespace kryon
{

class AuthenticationReceiver
{
public:

    AuthenticationReceiver() = default;

    void Receive(const AuthenticationPacket& packet)
    {
        Logger::Info(
            "[Receiver] Packet received : " +
            packet.requestId);
    }
};

}

#endif
