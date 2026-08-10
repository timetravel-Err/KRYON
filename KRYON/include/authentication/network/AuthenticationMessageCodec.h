#ifndef KRYON_AUTHENTICATION_MESSAGE_CODEC_H
#define KRYON_AUTHENTICATION_MESSAGE_CODEC_H

#include "../packets/AuthRequestPacket.h"

#include "ns3/packet.h"

#include <cstring>
#include <memory>

namespace kryon
{

class AuthenticationMessageCodec
{
public:

    static ns3::Ptr<ns3::Packet> Encode(
        const AuthRequestPacket& request)
    {
        uint32_t size = sizeof(AuthRequestPacket);

        uint8_t* buffer = new uint8_t[size];

        std::memcpy(buffer, &request, size);

        ns3::Ptr<ns3::Packet> packet =
            ns3::Create<ns3::Packet>(buffer, size);

        delete[] buffer;

        return packet;
    }

    static AuthRequestPacket DecodeRequest(
        ns3::Ptr<ns3::Packet> packet)
    {
        AuthRequestPacket request;

        packet->CopyData(
            reinterpret_cast<uint8_t*>(&request),
            sizeof(AuthRequestPacket));

        return request;
    }
};

}

#endif
