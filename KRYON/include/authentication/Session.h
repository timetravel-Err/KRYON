#ifndef KRYON_SESSION_H
#define KRYON_SESSION_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : Session.h
 *
 * Description
 * -----------
 * Represents an authenticated security session
 * established between a Drone and a Vehicle.
 *
 * The session is created after successful mutual
 * authentication and can later be reused to avoid
 * repeating the complete authentication protocol.
 *
 * Future versions may include:
 * • Session renewal
 * • Session revocation
 * • Perfect Forward Secrecy
 * • Blockchain-backed session validation
 * ----------------------------------------------------------
 */

#include <string>

#include "../crypto/CryptoTypes.h"

namespace kryon
{

struct Session
{
    /*
     * Unique session identifier.
     */
    std::string sessionId;

    /*
     * Drone participating in this session.
     */
    uint32_t droneId = 0;

    /*
     * Vehicle participating in this session.
     */
    uint32_t vehicleId = 0;

    /*
     * Shared session key derived using HKDF.
     */
    SessionKey sessionKey;

    /*
     * Simulation time when session was created.
     */
    double creationTime = 0.0;

    /*
     * Session expiration time.
     */
    double expirationTime = 0.0;

    /*
     * Indicates whether the session is valid.
     */
    bool active = false;
	
	/*
	 * Highest accepted secure-channel sequence number.
	 *
	 * Used for replay protection. A received packet is
	 * accepted only if its sequence number is greater
	 * than this value.
	 */
	uint64_t lastReceivedSequence = 0;
	
	/*
	 * Next sequence number to use for an outgoing
	 * secure-channel packet.
	 */
	uint64_t nextSendSequence = 1;
};

}

#endif
