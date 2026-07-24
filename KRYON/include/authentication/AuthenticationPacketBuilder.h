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

};

}

#endif
