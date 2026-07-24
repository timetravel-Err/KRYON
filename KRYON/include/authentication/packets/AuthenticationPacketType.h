#ifndef KRYON_AUTHENTICATION_PACKET_TYPE_H
#define KRYON_AUTHENTICATION_PACKET_TYPE_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : AuthenticationPacketType.h
 *
 * Description
 * -----------
 * Defines all authentication packet types exchanged
 * over the simulated network.
 * ----------------------------------------------------------
 */

namespace kryon
{

enum class AuthenticationPacketType
{
    AUTH_REQUEST = 1,

    AUTH_CHALLENGE,

    AUTH_RESPONSE,

    AUTH_CONFIRM,

    AUTH_ERROR
};

}

#endif
