#ifndef KRYON_AUTHENTICATION_PACKET_H
#define KRYON_AUTHENTICATION_PACKET_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : AuthenticationPacket.h
 *
 * Description
 * -----------
 * Base class for all authentication packets exchanged
 * during protocol execution.
 *
 * Every authentication packet inherits these common fields.
 * ----------------------------------------------------------
 */

#include <string>
#include <cstdint>

#include "AuthenticationPacketType.h"

namespace kryon
{

class AuthenticationPacket
{
public:

    virtual ~AuthenticationPacket() = default;

    AuthenticationPacketType type =
        AuthenticationPacketType::AUTH_REQUEST;

    std::string requestId;

    uint32_t sourceNode = 0;

    uint32_t destinationNode = 0;

    double timestamp = 0.0;

    virtual uint32_t GetPacketSize() const = 0;
};

}

#endif
