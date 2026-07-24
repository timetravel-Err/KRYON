#ifndef KRYON_AUTH_REQUEST_PACKET_H
#define KRYON_AUTH_REQUEST_PACKET_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : AuthRequestPacket.h
 *
 * Description
 * -----------
 * Represents the first authentication packet transmitted
 * from the requesting node to the responder.
 *
 * This packet contains generic fields that can be reused
 * by RAP, SLAP-IoAV, 2PQS-IoAV, TC2PA, DID and future
 * authentication protocols.
 * ----------------------------------------------------------
 */

#include "AuthenticationPacket.h"

#include <vector>
#include <cstdint>

namespace kryon
{

class AuthRequestPacket : public AuthenticationPacket
{
public:

    AuthRequestPacket()
    {
        type = AuthenticationPacketType::AUTH_REQUEST;
    }

    /* ---------- Freshness ---------- */

    std::vector<uint8_t> nonce;

    /* ---------- Authentication Data ---------- */
/*
 * Protocol-specific authentication information.
 *
 * Examples:
 *   RAP       -> ECC public key
 *   SLAP      -> PUF response
 *   2PQS      -> Kyber public key
 *   DID       -> Verifiable Credential
 *   Blockchain-> Transaction reference
 */
std::vector<uint8_t> authenticationData;

/* ---------- Integrity ---------- */
/*
 * Integrity hash protecting the authentication data.
 */
std::vector<uint8_t> integrityHash;

  

    uint32_t GetPacketSize() const override
    {
        return sizeof(type)
             + requestId.size()
             + sizeof(sourceNode)
             + sizeof(destinationNode)
             + sizeof(timestamp)
             + nonce.size()
			 + authenticationData.size()
			 + integrityHash.size();
    }
};

}

#endif
