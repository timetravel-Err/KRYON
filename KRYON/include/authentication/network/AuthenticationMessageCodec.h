#ifndef KRYON_AUTHENTICATION_MESSAGE_CODEC_H
#define KRYON_AUTHENTICATION_MESSAGE_CODEC_H

#include "../packets/AuthRequestPacket.h"
#include "../serialization/PacketWriter.h"
#include "../serialization/PacketReader.h"

#include "ns3/packet.h"

#include <vector>
#include <cstdint>
#include <stdexcept>

namespace kryon
{

class AuthenticationMessageCodec
{
public:

    /*
     * ------------------------------------------------------
     * Encode AuthRequestPacket
     * ------------------------------------------------------
     */

    static ns3::Ptr<ns3::Packet> Encode(
        const AuthRequestPacket& request)
    {
        PacketWriter writer;

        writer.Write(
            static_cast<uint8_t>(request.type));

        writer.WriteString(
            request.requestId);

        writer.Write(
            request.sourceNode);

        writer.Write(
            request.destinationNode);

        writer.Write(
            request.timestamp);

        writer.WriteVector(
            request.nonce);

        writer.WriteVector(
            request.authenticationData);

        writer.WriteVector(
            request.integrityHash);

        const std::vector<uint8_t>& buffer =
            writer.GetBuffer();

        return ns3::Create<ns3::Packet>(
            buffer.data(),
            buffer.size());
    }


    /*
     * ------------------------------------------------------
     * Decode AuthRequestPacket
     * ------------------------------------------------------
     */

    static AuthRequestPacket DecodeRequest(
        ns3::Ptr<ns3::Packet> packet)
    {
        std::vector<uint8_t> buffer(
            packet->GetSize());

        packet->CopyData(
            buffer.data(),
            buffer.size());

        PacketReader reader(buffer);

        AuthRequestPacket request;

        request.type =
            static_cast<AuthenticationPacketType>(
                reader.Read<uint8_t>());

        request.requestId =
            reader.ReadString();

        request.sourceNode =
            reader.Read<uint32_t>();

        request.destinationNode =
            reader.Read<uint32_t>();

        request.timestamp =
            reader.Read<double>();

        request.nonce =
            reader.ReadVector();

        request.authenticationData =
            reader.ReadVector();

        request.integrityHash =
            reader.ReadVector();

        return request;
    }
};

}

#endif