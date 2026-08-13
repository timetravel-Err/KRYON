#ifndef KRYON_AUTHENTICATION_MESSAGE_CODEC_H
#define KRYON_AUTHENTICATION_MESSAGE_CODEC_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : AuthenticationMessageCodec.h
 *
 * Description
 * -----------
 * Serialization/deserialization of authentication packets.
 *
 * Supported messages:
 *
 *   AUTH_REQUEST
 *   AUTH_CHALLENGE
 *   AUTH_RESPONSE
 *   AUTH_CONFIRM
 *
 * The codec converts protocol packet structures into ns-3
 * UDP payloads and reconstructs them at the receiver.
 * ----------------------------------------------------------
 */

#include "../packets/AuthRequestPacket.h"
#include "../packets/AuthChallengePacket.h"
#include "../packets/AuthResponsePacket.h"
#include "../packets/AuthConfirmPacket.h"

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

    /* ======================================================
     * AUTH_REQUEST
     * ====================================================== */

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

        const auto& buffer =
            writer.GetBuffer();

        return ns3::Create<ns3::Packet>(
            buffer.data(),
            buffer.size());
    }


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


    /* ======================================================
     * AUTH_CHALLENGE
     * ====================================================== */

    static ns3::Ptr<ns3::Packet> Encode(
        const AuthChallengePacket& challenge)
    {
        PacketWriter writer;

        writer.Write(
            static_cast<uint8_t>(challenge.type));

        writer.WriteString(
            challenge.requestId);

        writer.Write(
            challenge.sourceNode);

        writer.Write(
            challenge.destinationNode);

        writer.Write(
            challenge.timestamp);

        writer.WriteVector(
            challenge.challenge);

        writer.WriteVector(
            challenge.senderPublicKey);

        writer.WriteVector(
            challenge.signature);

        writer.WriteVector(
            challenge.integrityHash);

        const auto& buffer =
            writer.GetBuffer();

        return ns3::Create<ns3::Packet>(
            buffer.data(),
            buffer.size());
    }


    static AuthChallengePacket DecodeChallenge(
        ns3::Ptr<ns3::Packet> packet)
    {
        std::vector<uint8_t> buffer(
            packet->GetSize());

        packet->CopyData(
            buffer.data(),
            buffer.size());

        PacketReader reader(buffer);

        AuthChallengePacket challenge;

        challenge.type =
            static_cast<AuthenticationPacketType>(
                reader.Read<uint8_t>());

        challenge.requestId =
            reader.ReadString();

        challenge.sourceNode =
            reader.Read<uint32_t>();

        challenge.destinationNode =
            reader.Read<uint32_t>();

        challenge.timestamp =
            reader.Read<double>();

        challenge.challenge =
            reader.ReadVector();

        challenge.senderPublicKey =
            reader.ReadVector();

        challenge.signature =
            reader.ReadVector();

        challenge.integrityHash =
            reader.ReadVector();

        return challenge;
    }


    /* ======================================================
     * AUTH_RESPONSE
     * ====================================================== */

    static ns3::Ptr<ns3::Packet> Encode(
        const AuthResponsePacket& response)
    {
        PacketWriter writer;

        writer.Write(
            static_cast<uint8_t>(response.type));

        writer.WriteString(
            response.requestId);

        writer.Write(
            response.sourceNode);

        writer.Write(
            response.destinationNode);

        writer.Write(
            response.timestamp);

        writer.WriteVector(
            response.challenge);

        writer.WriteVector(
            response.proof);

        writer.WriteVector(
            response.senderPublicKey);

        writer.WriteVector(
            response.signature);

        writer.WriteVector(
            response.integrityHash);

        const auto& buffer =
            writer.GetBuffer();

        return ns3::Create<ns3::Packet>(
            buffer.data(),
            buffer.size());
    }


    static AuthResponsePacket DecodeResponse(
        ns3::Ptr<ns3::Packet> packet)
    {
        std::vector<uint8_t> buffer(
            packet->GetSize());

        packet->CopyData(
            buffer.data(),
            buffer.size());

        PacketReader reader(buffer);

        AuthResponsePacket response;

        response.type =
            static_cast<AuthenticationPacketType>(
                reader.Read<uint8_t>());

        response.requestId =
            reader.ReadString();

        response.sourceNode =
            reader.Read<uint32_t>();

        response.destinationNode =
            reader.Read<uint32_t>();

        response.timestamp =
            reader.Read<double>();

        response.challenge =
            reader.ReadVector();

        response.proof =
            reader.ReadVector();

        response.senderPublicKey =
            reader.ReadVector();

        response.signature =
            reader.ReadVector();

        response.integrityHash =
            reader.ReadVector();

        return response;
    }


    /* ======================================================
     * AUTH_CONFIRM
     * ====================================================== */

    static ns3::Ptr<ns3::Packet> Encode(
        const AuthConfirmPacket& confirm)
    {
        PacketWriter writer;

        writer.Write(
            static_cast<uint8_t>(confirm.type));

        writer.WriteString(
            confirm.requestId);

        writer.Write(
            confirm.sourceNode);

        writer.Write(
            confirm.destinationNode);

        writer.Write(
            confirm.timestamp);

        writer.Write(
            confirm.authenticationSuccessful);

        writer.WriteString(
            confirm.message);

        writer.WriteVector(
            confirm.integrityHash);

        const auto& buffer =
            writer.GetBuffer();

        return ns3::Create<ns3::Packet>(
            buffer.data(),
            buffer.size());
    }


    static AuthConfirmPacket DecodeConfirm(
        ns3::Ptr<ns3::Packet> packet)
    {
        std::vector<uint8_t> buffer(
            packet->GetSize());

        packet->CopyData(
            buffer.data(),
            buffer.size());

        PacketReader reader(buffer);

        AuthConfirmPacket confirm;

        confirm.type =
            static_cast<AuthenticationPacketType>(
                reader.Read<uint8_t>());

        confirm.requestId =
            reader.ReadString();

        confirm.sourceNode =
            reader.Read<uint32_t>();

        confirm.destinationNode =
            reader.Read<uint32_t>();

        confirm.timestamp =
            reader.Read<double>();

        confirm.authenticationSuccessful =
            reader.Read<bool>();

        confirm.message =
            reader.ReadString();

        confirm.integrityHash =
            reader.ReadVector();

        return confirm;
    }
};

}

#endif