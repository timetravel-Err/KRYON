#ifndef KRYON_AUTHENTICATION_PACKET_BUILDER_H
#define KRYON_AUTHENTICATION_PACKET_BUILDER_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : AuthenticationPacketBuilder.h
 *
 * Description
 * -----------
 * Converts framework authentication objects into
 * network packets exchanged over ns-3.
 *
 * This builder isolates packet creation from protocol
 * implementations.
 * ----------------------------------------------------------
 */

#include "AuthenticationRequest.h"
#include "AuthenticationChallenge.h"
#include "AuthenticationResponse.h"
#include "AuthenticationResult.h"

#include "packets/AuthRequestPacket.h"
#include "packets/AuthChallengePacket.h"
#include "packets/AuthResponsePacket.h"
#include "packets/AuthConfirmPacket.h"

namespace kryon
{

class AuthenticationPacketBuilder
{
public:

    AuthenticationPacketBuilder() = default;

    AuthRequestPacket BuildRequest(
        const AuthenticationRequest& request)
    {
        AuthRequestPacket packet;

        packet.requestId       = request.requestId;
        packet.sourceNode      = request.sourceNodeId;
        packet.destinationNode = request.destinationNodeId;
        packet.timestamp       = request.timestamp;

        return packet;
    }

    AuthChallengePacket BuildChallenge(
        const AuthenticationChallenge& challenge)
    {
        
    AuthChallengePacket packet;

    packet.requestId       = challenge.requestId;
    packet.sourceNode      = challenge.sourceNodeId;
    packet.destinationNode = challenge.destinationNodeId;
    packet.timestamp       = challenge.timestamp;

    packet.challenge = challenge.challenge;

    return packet;

    }

    AuthResponsePacket BuildResponse(
        const AuthenticationResponse& response)
    {
        AuthResponsePacket packet;

        packet.requestId = response.requestId;

        return packet;
    }

    AuthConfirmPacket BuildConfirmation(
        const AuthenticationResult& result)
    {
       AuthConfirmPacket packet;

    packet.requestId = result.requestId;

    packet.authenticationSuccessful =
        result.authenticated;

    packet.message = result.reason;

    return packet;
    }

};

}

#endif
