#ifndef KRYON_AUTHENTICATION_RECEIVER_H
#define KRYON_AUTHENTICATION_RECEIVER_H

#include "../packets/AuthenticationPacket.h"
#include "AuthenticationDispatcher.h"

#include "../../core/Logger.h"

namespace kryon
{

class AuthenticationReceiver
{
public:

    AuthenticationReceiver() = default;

    void SetDispatcher(
        AuthenticationDispatcher* dispatcher)
    {
        m_dispatcher = dispatcher;
    }

    void Receive(
        const AuthenticationPacket& packet)
    {
        Logger::Info(
            "[Receiver] Packet received : " +
            packet.requestId);

        if (m_dispatcher != nullptr)
        {
            m_dispatcher->Dispatch(packet);
        }
    }

private:

    AuthenticationDispatcher* m_dispatcher = nullptr;
};

}

#endif