#include "../../KRYON/include/authentication/protocols/rap/RAPAuthenticationProtocol.h"
#include "../../KRYON/include/authentication/AuthenticationRequest.h"
#include "../../KRYON/include/authentication/SessionManager.h"
#include "../../KRYON/include/crypto/CryptoEngine.h"
#include "../../KRYON/include/security/SecureChannel.h"
#include "../../KRYON/include/security/SecurePacket.h"
#include "../../KRYON/include/core/ExperimentConfig.h"
#include "../../KRYON/include/simulation/SimulationContext.h"

#include <iostream>
#include <string>
#include <stdexcept>
#include "ns3/simulator.h"

int main()
{
    std::cout
        << "=========================================="
        << std::endl;

    std::cout
        << "KRYON RAP + SECURE CHANNEL INTEGRATION TEST"
        << std::endl;

    std::cout
        << "=========================================="
        << std::endl;

    /*
     * ------------------------------------------------------
     * Configuration and simulation context
     * ------------------------------------------------------
     */

    kryon::ExperimentConfig config;
    kryon::SimulationContext context;

    /*
     * ------------------------------------------------------
     * Session Manager
     * ------------------------------------------------------
     */

    kryon::SessionManager sessionManager(
        config,
        context);

    sessionManager.Initialize();

    /*
     * ------------------------------------------------------
     * Crypto Engine
     * ------------------------------------------------------
     */

    kryon::CryptoEngine crypto(
        config,
        context);

    crypto.Initialize();

    /*
     * ------------------------------------------------------
     * RAP Authentication Protocol
     * ------------------------------------------------------
     */

    kryon::RAPAuthenticationProtocol rap;

    rap.SetCryptoEngine(&crypto);
    rap.SetSessionManager(&sessionManager);

    rap.Initialize();

    /*
     * ------------------------------------------------------
     * Secure Channel
     * ------------------------------------------------------
     */

    kryon::SecureChannel secureChannel;

    secureChannel.SetCryptoEngine(&crypto);
    secureChannel.SetSessionManager(&sessionManager);

    secureChannel.Initialize();

    /*
     * ------------------------------------------------------
     * TEST 1
     * RAP AUTHENTICATION
     * ------------------------------------------------------
     */

    std::cout
        << "\n------------------------------------------"
        << std::endl;

    std::cout
        << "TEST 1: RAP AUTHENTICATION"
        << std::endl;

    std::cout
        << "------------------------------------------"
        << std::endl;

    kryon::AuthenticationRequest request;

    request.requestId =
        "RAP-SECURE-INTEGRATION-001";

    request.sourceNodeId =
        1;

    request.destinationNodeId =
        2;

    request.timestamp =
        1.0;

    request.method =
        kryon::AuthenticationMethod::ECC;

    kryon::AuthenticationResult authResult =
        rap.Authenticate(request);

    if (!authResult.authenticated)
    {
        std::cout
            << "RAP AUTHENTICATION: FAILED"
            << std::endl;

        secureChannel.Finalize();
        rap.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    std::cout
        << "Authentication : SUCCESS"
        << std::endl;

    std::cout
        << "Session ID     : "
        << authResult.sessionId
        << std::endl;

    std::cout
        << "Session key    : "
        << authResult.sessionKey.bytes.data.size()
        << " bytes"
        << std::endl;

    if (authResult.sessionKey.bytes.data.size() != 32)
    {
        std::cout
            << "SESSION KEY SIZE: FAILED"
            << std::endl;

        secureChannel.Finalize();
        rap.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    /*
     * ------------------------------------------------------
     * Obtain authenticated session
     * ------------------------------------------------------
     */

    kryon::Session* session =
        sessionManager.FindSession(
            1,
            2,
            1.0);

    if (session == nullptr)
    {
        std::cout
            << "SESSION CREATION: FAILED"
            << std::endl;

        secureChannel.Finalize();
        rap.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    std::cout
        << "Authenticated session obtained."
        << std::endl;

    /*
     * ------------------------------------------------------
     * TEST 2
     * SECURE CHANNEL ENCRYPTION
     * ------------------------------------------------------
     */

    std::cout
        << "\n------------------------------------------"
        << std::endl;

    std::cout
        << "TEST 2: SECURE CHANNEL ENCRYPTION"
        << std::endl;

    std::cout
        << "------------------------------------------"
        << std::endl;

    const std::string message =
        "KRYON secure application message";

    kryon::ByteArray plaintext;

    plaintext.data.assign(
        message.begin(),
        message.end());

    kryon::SecurePacket packet =
        secureChannel.Encrypt(
            plaintext,
            *session);

    std::cout
        << "Plaintext size : "
        << plaintext.data.size()
        << " bytes"
        << std::endl;

    std::cout
        << "Ciphertext size: "
        << packet.ciphertext.data.size()
        << " bytes"
        << std::endl;

    std::cout
        << "Session ID     : "
        << packet.sessionId
        << std::endl;

    std::cout
        << "Sequence       : "
        << packet.sequenceNumber
        << std::endl;

    std::cout
        << "Nonce size     : "
        << packet.nonce.data.size()
        << " bytes"
        << std::endl;

    std::cout
        << "GCM tag size   : "
        << packet.mac.data.size()
        << " bytes"
        << std::endl;

    if (!packet.encrypted ||
        packet.ciphertext.data.empty() ||
        packet.nonce.data.size() != 12 ||
        packet.mac.data.size() != 16)
    {
        std::cout
            << "SECURE ENCRYPTION: FAILED"
            << std::endl;

        secureChannel.Finalize();
        rap.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    std::cout
        << "SECURE ENCRYPTION: SUCCESS"
        << std::endl;

    /*
     * ------------------------------------------------------
     * TEST 3
     * SECURE CHANNEL DECRYPTION
     * ------------------------------------------------------
     */

    std::cout
        << "\n------------------------------------------"
        << std::endl;

    std::cout
        << "TEST 3: SECURE CHANNEL DECRYPTION"
        << std::endl;

    std::cout
        << "------------------------------------------"
        << std::endl;

    /*
     * Use a fresh receiver session object so that
     * outgoing sequence state does not interfere with
     * receiver replay state.
     */
    kryon::Session receiverSession =
        *session;

    kryon::ByteArray recovered;

    try
    {
        recovered =
            secureChannel.Decrypt(
                packet,
                receiverSession);
    }
    catch (const std::exception& e)
    {
        std::cout
            << "SECURE DECRYPTION: FAILED"
            << std::endl;

        std::cout
            << "Reason: "
            << e.what()
            << std::endl;

        secureChannel.Finalize();
        rap.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    std::string recoveredMessage(
        recovered.data.begin(),
        recovered.data.end());

    std::cout
        << "Recovered message: "
        << recoveredMessage
        << std::endl;

    if (recoveredMessage != message)
    {
        std::cout
            << "MESSAGE RECOVERY: FAILED"
            << std::endl;

        secureChannel.Finalize();
        rap.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    std::cout
        << "SECURE DECRYPTION: SUCCESS"
        << std::endl;

    /*
     * ------------------------------------------------------
     * TEST 4
     * REPLAY PROTECTION
     * ------------------------------------------------------
     */

    std::cout
        << "\n------------------------------------------"
        << std::endl;

    std::cout
        << "TEST 4: REPLAY PROTECTION"
        << std::endl;

    std::cout
        << "------------------------------------------"
        << std::endl;

    try
    {
        secureChannel.Decrypt(
            packet,
            receiverSession);

        std::cout
            << "REPLAY PROTECTION: FAILED"
            << std::endl;

        secureChannel.Finalize();
        rap.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }
    catch (const std::exception& e)
    {
        std::cout
            << "Replay rejected: SUCCESS"
            << std::endl;

        std::cout
            << "Reason: "
            << e.what()
            << std::endl;
    }

    /*
     * ------------------------------------------------------
     * TEST 5
     * SESSION EXPIRATION
     * ------------------------------------------------------
     */

    std::cout
        << "\n------------------------------------------"
        << std::endl;

    std::cout
        << "TEST 5: SESSION EXPIRATION"
        << std::endl;

    std::cout
        << "------------------------------------------"
        << std::endl;

    const double expirationTime =
        1000.0;

    sessionManager.RemoveExpiredSessions(
        expirationTime);

    kryon::Session* expiredSession =
        sessionManager.FindSession(
            1,
            2,
            expirationTime);

    if (expiredSession != nullptr)
    {
        std::cout
            << "SESSION EXPIRATION: FAILED"
            << std::endl;

        secureChannel.Finalize();
        rap.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    std::cout
        << "SESSION EXPIRATION: SUCCESS"
        << std::endl;

    /*
     * ------------------------------------------------------
     * TEST 6
     * SECURE CHANNEL REJECTS EXPIRED SESSION
     * ------------------------------------------------------
     */

    std::cout
        << "\n------------------------------------------"
        << std::endl;

    std::cout
        << "TEST 6: EXPIRED SESSION SECURE CHANNEL"
        << std::endl;

    std::cout
        << "------------------------------------------"
        << std::endl;

    try
    {
        secureChannel.Encrypt(
            plaintext,
            receiverSession);

        std::cout
            << "EXPIRED SESSION REJECTION: FAILED"
            << std::endl;

        secureChannel.Finalize();
        rap.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }
    catch (const std::exception& e)
    {
        std::cout
            << "Expired session rejected: SUCCESS"
            << std::endl;

        std::cout
            << "Reason: "
            << e.what()
            << std::endl;
    }
	
	/*
	 * ------------------------------------------------------
	 * Advance simulation time to the re-authentication point.
	 *
	 * RAP uses Simulator::Now() when creating the new session.
	 * ------------------------------------------------------
	 */

	ns3::Simulator::Schedule(
		ns3::Seconds(expirationTime),
		[]()
		{
			std::cout
				<< "[INFO] Simulation time advanced to re-authentication point."
				<< std::endl;
		});

	ns3::Simulator::Run();

    /*
     * ------------------------------------------------------
     * TEST 7
     * RE-AUTHENTICATION
     * ------------------------------------------------------
     */

    std::cout
        << "\n------------------------------------------"
        << std::endl;

    std::cout
        << "TEST 7: RAP RE-AUTHENTICATION"
        << std::endl;

    std::cout
        << "------------------------------------------"
        << std::endl;

    kryon::AuthenticationRequest reauthRequest;

    reauthRequest.requestId =
        "RAP-SECURE-INTEGRATION-002";

    reauthRequest.sourceNodeId =
        1;

    reauthRequest.destinationNodeId =
        2;

    reauthRequest.timestamp =
        expirationTime;

    reauthRequest.method =
        kryon::AuthenticationMethod::ECC;

    kryon::AuthenticationResult reauthResult =
        rap.Authenticate(
            reauthRequest);

    if (!reauthResult.authenticated)
    {
        std::cout
            << "RE-AUTHENTICATION: FAILED"
            << std::endl;

        secureChannel.Finalize();
        rap.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    std::cout
        << "Re-authentication : SUCCESS"
        << std::endl;

    std::cout
        << "New Session ID    : "
        << reauthResult.sessionId
        << std::endl;

    std::cout
        << "New Session key   : "
        << reauthResult.sessionKey.bytes.data.size()
        << " bytes"
        << std::endl;

    if (reauthResult.sessionId ==
        authResult.sessionId)
    {
        std::cout
            << "NEW SESSION ID: FAILED"
            << std::endl;

        secureChannel.Finalize();
        rap.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    if (reauthResult.sessionKey.bytes.data.size() != 32)
    {
        std::cout
            << "NEW SESSION KEY: FAILED"
            << std::endl;

        secureChannel.Finalize();
        rap.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    /*
     * ------------------------------------------------------
     * Obtain new authenticated session
     * ------------------------------------------------------
     */

    kryon::Session* newSession =
        sessionManager.FindSession(
            1,
            2,
            expirationTime);

    if (newSession == nullptr)
    {
        std::cout
            << "NEW SESSION CREATION: FAILED"
            << std::endl;

        secureChannel.Finalize();
        rap.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    /*
     * ------------------------------------------------------
     * TEST 8
     * SECURE COMMUNICATION AFTER RE-AUTHENTICATION
     * ------------------------------------------------------
     */

    std::cout
        << "\n------------------------------------------"
        << std::endl;

    std::cout
        << "TEST 8: SECURE COMMUNICATION AFTER REAUTH"
        << std::endl;

    std::cout
        << "------------------------------------------"
        << std::endl;

    const std::string newMessage =
        "KRYON message after re-authentication";

    kryon::ByteArray newPlaintext;

    newPlaintext.data.assign(
        newMessage.begin(),
        newMessage.end());

    kryon::SecurePacket newPacket =
        secureChannel.Encrypt(
            newPlaintext,
            *newSession);

    kryon::ByteArray newRecovered;

    try
    {
        newRecovered =
            secureChannel.Decrypt(
                newPacket,
                *newSession);
    }
    catch (const std::exception& e)
    {
        std::cout
            << "POST-REAUTH SECURE CHANNEL: FAILED"
            << std::endl;

        std::cout
            << "Reason: "
            << e.what()
            << std::endl;

        secureChannel.Finalize();
        rap.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    std::string newRecoveredMessage(
        newRecovered.data.begin(),
        newRecovered.data.end());

    std::cout
        << "Recovered message: "
        << newRecoveredMessage
        << std::endl;

    if (newRecoveredMessage != newMessage)
    {
        std::cout
            << "POST-REAUTH MESSAGE: FAILED"
            << std::endl;

        secureChannel.Finalize();
        rap.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    std::cout
        << "POST-REAUTH SECURE CHANNEL: SUCCESS"
        << std::endl;

    /*
     * ------------------------------------------------------
     * Finalization
     * ------------------------------------------------------
     */

    secureChannel.Finalize();
    rap.Finalize();
    sessionManager.Finalize();
    crypto.Finalize();

    std::cout
        << "\n=========================================="
        << std::endl;

    std::cout
        << "RAP + SECURE CHANNEL INTEGRATION PASSED"
        << std::endl;

    std::cout
        << "=========================================="
        << std::endl;

    return 0;
}
