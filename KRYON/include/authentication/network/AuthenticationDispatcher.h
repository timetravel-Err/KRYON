#ifndef KRYON_AUTHENTICATION_DISPATCHER_H
#define KRYON_AUTHENTICATION_DISPATCHER_H

#include "../packets/AuthenticationPacket.h"
#include "../../core/Logger.h"

namespace kryon
{

class AuthenticationDispatcher
{
public:

    AuthenticationDispatcher() = default;

    void Dispatch(const AuthenticationPacket& packet)
    {
        Logger::Info(
            "[Dispatcher] Dispatching packet : " +
            packet.requestId);
    }
};

}

#endif
