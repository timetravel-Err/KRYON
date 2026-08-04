#ifndef KRYON_AUTHENTICATION_RESPONSE_H
#define KRYON_AUTHENTICATION_RESPONSE_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : AuthenticationResponse.h
 *
 * Description
 * -----------
 * Represents the response returned after processing an
 * authentication request.
 *
 * This structure is protocol-independent and is intended
 * to support future authentication mechanisms.
 * ----------------------------------------------------------
 */
#include <string>
#include <vector>
#include <cstdint>

#include "AuthenticationTypes.h"
#include "../crypto/CryptoTypes.h"

namespace kryon
{

struct AuthenticationResponse
{
    std::string requestId;

    uint32_t responderNodeId = 0;

    /*
 * Challenge received from verifier.
 */
	std::vector<uint8_t> challenge;

    /*
 * Protocol-specific proof.
 *
 * Examples:
 *   RAP       -> SHA-256 response
 *   SLAP      -> PUF response
 *   2PQS      -> PQ signature
 *   DID       -> Verifiable Presentation
 */
	std::vector<uint8_t> proof;
	
	/*
 * Sender's public key.
 *
 * Used for ECDSA signature verification.
 */
	PublicKey senderPublicKey;

/*
 * Digital signature over the authentication response.
 *
 * Generated using the sender's private ECC key.
 */
	Signature signature;

    double timestamp = 0.0;
};	
}

#endif
