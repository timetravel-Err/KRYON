#include "../../KRYON/include/crypto/CryptoEngine.h"
#include "../../KRYON/include/core/ExperimentConfig.h"
#include "../../KRYON/include/simulation/SimulationContext.h"
#include "../../KRYON/include/core/Logger.h"
#include "../../KRYON/include/authentication/SessionManager.h"
#include "../../KRYON/include/security/SecureChannel.h"

#include <iostream>
#include <string>

int main()
{
    std::cout
        << "=========================================="
        << std::endl;

    std::cout
        << "KRYON SECURE CHANNEL OUT-OF-ORDER TEST"
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
     * Secure Channel
     * ------------------------------------------------------
     */

    kryon::SecureChannel secureChannel;

    secureChannel.SetCryptoEngine(&crypto);
    secureChannel.SetSessionManager(&sessionManager);

    secureChannel.Initialize();

    /*
     * ------------------------------------------------------
     * Create test session
     * ------------------------------------------------------
     */

    kryon::Session session;

    session.sessionId =
        "KRYON-SESSION-1-2-OUT-OF-ORDER-TEST";

    session.droneId =
        1;

    session.vehicleId =
        2;

    session.creationTime =
        0.0;

    session.expirationTime =
        600.0;

    session.active =
        true;

    /*
     * Create a deterministic 32-byte test session key.
     *
     * This is only for the standalone secure-channel
     * test. The production RAP flow derives the key
     * using ECDH + HKDF.
     */

    session.sessionKey.bytes.data.resize(32);

    for (std::size_t i = 0;
         i < session.sessionKey.bytes.data.size();
         ++i)
    {
        session.sessionKey.bytes.data[i] =
            static_cast<unsigned char>(i + 1);
    }

    sessionManager.CreateSession(session);

    std::cout
        << "\n------------------------------------------"
        << std::endl;

    std::cout
        << "SESSION SETUP"
        << std::endl;

    std::cout
        << "------------------------------------------"
        << std::endl;

    std::cout
        << "Session ID      : "
        << session.sessionId
        << std::endl;

    std::cout
        << "Session key size: "
        << session.sessionKey.bytes.data.size()
        << " bytes"
        << std::endl;

    /*
     * ------------------------------------------------------
     * Create four encrypted packets.
     * ------------------------------------------------------
     */

    kryon::ByteArray message1;
    std::string text1 = "Packet1";

    message1.data.assign(
        text1.begin(),
        text1.end());

    kryon::ByteArray message2;
    std::string text2 = "Packet2";

    message2.data.assign(
        text2.begin(),
        text2.end());

    kryon::ByteArray message3;
    std::string text3 = "Packet3";

    message3.data.assign(
        text3.begin(),
        text3.end());

    kryon::ByteArray message4;
    std::string text4 = "Packet4";

    message4.data.assign(
        text4.begin(),
        text4.end());

    kryon::SecurePacket packet1 =
        secureChannel.Encrypt(
            message1,
            session);

    kryon::SecurePacket packet2 =
        secureChannel.Encrypt(
            message2,
            session);

    kryon::SecurePacket packet3 =
        secureChannel.Encrypt(
            message3,
            session);

    kryon::SecurePacket packet4 =
        secureChannel.Encrypt(
            message4,
            session);

    /*
     * ------------------------------------------------------
     * TEST 1
     * Verify sequence allocation.
     * ------------------------------------------------------
     */

    std::cout
        << "\n------------------------------------------"
        << std::endl;

    std::cout
        << "TEST 1: SEQUENCE NUMBER ALLOCATION"
        << std::endl;

    std::cout
        << "------------------------------------------"
        << std::endl;

    std::cout
        << "Packet 1 sequence: "
        << packet1.sequenceNumber
        << std::endl;

    std::cout
        << "Packet 2 sequence: "
        << packet2.sequenceNumber
        << std::endl;

    std::cout
        << "Packet 3 sequence: "
        << packet3.sequenceNumber
        << std::endl;

    std::cout
        << "Packet 4 sequence: "
        << packet4.sequenceNumber
        << std::endl;

    if (packet1.sequenceNumber != 1 ||
        packet2.sequenceNumber != 2 ||
        packet3.sequenceNumber != 3 ||
        packet4.sequenceNumber != 4)
    {
        std::cout
            << "\nSEQUENCE ALLOCATION: FAILED"
            << std::endl;

        secureChannel.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    std::cout
        << "SEQUENCE ALLOCATION: SUCCESS"
        << std::endl;

    /*
     * ------------------------------------------------------
     * TEST 2
     * Packet 1 accepted.
     * ------------------------------------------------------
     */

    std::cout
        << "\n------------------------------------------"
        << std::endl;

    std::cout
        << "TEST 2: PACKET 1 ACCEPTED"
        << std::endl;

    std::cout
        << "------------------------------------------"
        << std::endl;

    try
    {
        kryon::ByteArray recovered =
            secureChannel.Decrypt(
                packet1,
                session);

        std::string recoveredText(
            recovered.data.begin(),
            recovered.data.end());

        std::cout
            << "Recovered message: "
            << recoveredText
            << std::endl;

        if (recoveredText != "Packet1")
        {
            throw std::runtime_error(
                "Unexpected recovered message.");
        }

        std::cout
            << "PACKET 1: SUCCESS"
            << std::endl;
    }
    catch (const std::exception& error)
    {
        std::cout
            << "PACKET 1: FAILED"
            << std::endl;

        std::cout
            << "Reason: "
            << error.what()
            << std::endl;

        secureChannel.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    /*
     * ------------------------------------------------------
     * TEST 3
     * Packet 3 arrives before Packet 2.
     *
     * Packet 3 is authenticated and therefore accepted.
     * Replay state becomes 3.
     * ------------------------------------------------------
     */

    std::cout
        << "\n------------------------------------------"
        << std::endl;

    std::cout
        << "TEST 3: PACKET 3 ACCEPTED BEFORE PACKET 2"
        << std::endl;

    std::cout
        << "------------------------------------------"
        << std::endl;

    try
    {
        kryon::ByteArray recovered =
            secureChannel.Decrypt(
                packet3,
                session);

        std::string recoveredText(
            recovered.data.begin(),
            recovered.data.end());

        std::cout
            << "Recovered message: "
            << recoveredText
            << std::endl;

        if (recoveredText != "Packet3")
        {
            throw std::runtime_error(
                "Unexpected recovered message.");
        }

        std::cout
            << "PACKET 3: SUCCESS"
            << std::endl;
    }
    catch (const std::exception& error)
    {
        std::cout
            << "PACKET 3: FAILED"
            << std::endl;

        std::cout
            << "Reason: "
            << error.what()
            << std::endl;

        secureChannel.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    /*
     * ------------------------------------------------------
     * TEST 4
     * Packet 2 now arrives.
     *
     * Highest accepted sequence is already 3.
     *
     * Therefore sequence 2 must be rejected.
     * ------------------------------------------------------
     */

    std::cout
        << "\n------------------------------------------"
        << std::endl;

    std::cout
        << "TEST 4: PACKET 2 REJECTED AS OUT-OF-ORDER"
        << std::endl;

    std::cout
        << "------------------------------------------"
        << std::endl;

    try
    {
        secureChannel.Decrypt(
            packet2,
            session);

        std::cout
            << "PACKET 2 OUT-OF-ORDER: FAILED"
            << std::endl;

        std::cout
            << "ERROR: Packet 2 was accepted after Packet 3."
            << std::endl;

        secureChannel.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }
    catch (const std::exception& error)
    {
        std::cout
            << "Packet 2 rejected: SUCCESS"
            << std::endl;

        std::cout
            << "Reason: "
            << error.what()
            << std::endl;
    }

    /*
     * ------------------------------------------------------
     * TEST 5
     * Packet 4 arrives after Packet 3.
     *
     * Sequence 4 is greater than 3,
     * therefore it must be accepted.
     * ------------------------------------------------------
     */

    std::cout
        << "\n------------------------------------------"
        << std::endl;

    std::cout
        << "TEST 5: PACKET 4 ACCEPTED"
        << std::endl;

    std::cout
        << "------------------------------------------"
        << std::endl;

    try
    {
        kryon::ByteArray recovered =
            secureChannel.Decrypt(
                packet4,
                session);

        std::string recoveredText(
            recovered.data.begin(),
            recovered.data.end());

        std::cout
            << "Recovered message: "
            << recoveredText
            << std::endl;

        if (recoveredText != "Packet4")
        {
            throw std::runtime_error(
                "Unexpected recovered message.");
        }

        std::cout
            << "PACKET 4: SUCCESS"
            << std::endl;
    }
    catch (const std::exception& error)
    {
        std::cout
            << "PACKET 4: FAILED"
            << std::endl;

        std::cout
            << "Reason: "
            << error.what()
            << std::endl;

        secureChannel.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    /*
     * ------------------------------------------------------
     * TEST 6
     * Verify replay state.
     *
     * Packet 4 was the highest accepted packet.
     * Therefore Packet 2 and Packet 3 must remain rejected.
     * ------------------------------------------------------
     */

    std::cout
        << "\n------------------------------------------"
        << std::endl;

    std::cout
        << "TEST 6: REPLAY STATE AFTER OUT-OF-ORDER PACKET"
        << std::endl;

    std::cout
        << "------------------------------------------"
        << std::endl;

    try
    {
        secureChannel.Decrypt(
            packet2,
            session);

        std::cout
            << "REPLAY STATE: FAILED"
            << std::endl;

        secureChannel.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }
    catch (const std::exception& error)
    {
        std::cout
            << "Old Packet 2 rejected: SUCCESS"
            << std::endl;

        std::cout
            << "Reason: "
            << error.what()
            << std::endl;
    }

    try
    {
        secureChannel.Decrypt(
            packet3,
            session);

        std::cout
            << "REPLAY STATE: FAILED"
            << std::endl;

        secureChannel.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }
    catch (const std::exception& error)
    {
        std::cout
            << "Old Packet 3 rejected: SUCCESS"
            << std::endl;

        std::cout
            << "Reason: "
            << error.what()
            << std::endl;
    }

    /*
     * ------------------------------------------------------
     * Finalize
     * ------------------------------------------------------
     */

    secureChannel.Finalize();
    sessionManager.Finalize();
    crypto.Finalize();

    std::cout
        << "\n=========================================="
        << std::endl;

    std::cout
        << "ALL OUT-OF-ORDER TESTS PASSED"
        << std::endl;

    std::cout
        << "=========================================="
        << std::endl;

    return 0;
}
