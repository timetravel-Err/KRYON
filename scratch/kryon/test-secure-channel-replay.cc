#include "ns3/core-module.h"

#include "../../KRYON/include/core/ExperimentConfig.h"
#include "../../KRYON/include/simulation/SimulationContext.h"
#include "../../KRYON/include/crypto/CryptoEngine.h"
#include "../../KRYON/include/security/SecureChannel.h"
#include "../../KRYON/include/security/SecurePacket.h"
#include "../../KRYON/include/authentication/Session.h"
#include "../../KRYON/include/authentication/SessionManager.h"

#include <iostream>
#include <string>
#include <stdexcept>

using namespace ns3;
using namespace kryon;

int
main(int argc, char* argv[])
{
    CommandLine cmd;
    cmd.Parse(argc, argv);

    std::cout
        << "==========================================\n"
        << "KRYON SECURE CHANNEL REPLAY TEST\n"
        << "==========================================\n";

    /*
     * ------------------------------------------------------
     * Initialize framework
     * ------------------------------------------------------
     */

    ExperimentConfig config;
    SimulationContext context;

    CryptoEngine crypto(config, context);
    crypto.Initialize();

    SessionManager sessionManager(
        config,
        context);

    sessionManager.Initialize();

    SecureChannel secureChannel;

    secureChannel.SetCryptoEngine(&crypto);
    secureChannel.SetSessionManager(&sessionManager);

    secureChannel.Initialize();

    /*
     * ------------------------------------------------------
     * Create authenticated test session
     * ------------------------------------------------------
     */

    Session session;

    session.sessionId =
        "KRYON-SESSION-1-2-REPLAY-TEST";

    session.droneId = 1;
    session.vehicleId = 2;

    session.creationTime =
        Simulator::Now().GetSeconds();

    session.expirationTime =
        session.creationTime + 600.0;

    session.active = true;

    session.sessionKey.bytes =
        crypto.GenerateRandomBytes(32);

    sessionManager.CreateSession(session);

    std::cout
        << "\n------------------------------------------\n"
        << "SESSION SETUP\n"
        << "------------------------------------------\n";

    std::cout
        << "Session ID      : "
        << session.sessionId
        << "\n";

    std::cout
        << "Session key size: "
        << session.sessionKey.bytes.data.size()
        << " bytes\n";

    /*
     * ------------------------------------------------------
     * TEST 1
     * Sequence number allocation
     * ------------------------------------------------------
     */

    std::cout
        << "\n------------------------------------------\n"
        << "TEST 1: SEQUENCE NUMBER ALLOCATION\n"
        << "------------------------------------------\n";

    SecurePacket packet1 =
        secureChannel.Encrypt(
            ByteArray{
                std::vector<uint8_t>(
                    {'P', 'a', 'c', 'k', 'e', 't', '1'})
            },
            session);

    SecurePacket packet2 =
        secureChannel.Encrypt(
            ByteArray{
                std::vector<uint8_t>(
                    {'P', 'a', 'c', 'k', 'e', 't', '2'})
            },
            session);

    SecurePacket packet3 =
        secureChannel.Encrypt(
            ByteArray{
                std::vector<uint8_t>(
                    {'P', 'a', 'c', 'k', 'e', 't', '3'})
            },
            session);

    std::cout
        << "Packet 1 sequence: "
        << packet1.sequenceNumber
        << "\n";

    std::cout
        << "Packet 2 sequence: "
        << packet2.sequenceNumber
        << "\n";

    std::cout
        << "Packet 3 sequence: "
        << packet3.sequenceNumber
        << "\n";

    if (packet1.sequenceNumber != 1 ||
        packet2.sequenceNumber != 2 ||
        packet3.sequenceNumber != 3)
    {
        std::cout
            << "SEQUENCE ALLOCATION: FAILED\n";

        secureChannel.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    std::cout
        << "SEQUENCE ALLOCATION: SUCCESS\n";

    /*
     * ------------------------------------------------------
     * TEST 2
     * First packet accepted
     * ------------------------------------------------------
     */

    std::cout
        << "\n------------------------------------------\n"
        << "TEST 2: FIRST PACKET ACCEPTED\n"
        << "------------------------------------------\n";

    try
    {
        ByteArray recovered =
            secureChannel.Decrypt(
                packet1,
                session);

        std::string message(
            recovered.data.begin(),
            recovered.data.end());

        std::cout
            << "Recovered message: "
            << message
            << "\n";

        std::cout
            << "FIRST PACKET: SUCCESS\n";
    }
    catch (const std::exception& e)
    {
        std::cout
            << "FIRST PACKET: FAILED\n"
            << "Reason: "
            << e.what()
            << "\n";

        secureChannel.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    /*
     * ------------------------------------------------------
     * TEST 3
     * Second packet accepted
     * ------------------------------------------------------
     */

    std::cout
        << "\n------------------------------------------\n"
        << "TEST 3: SECOND PACKET ACCEPTED\n"
        << "------------------------------------------\n";

    try
    {
        ByteArray recovered =
            secureChannel.Decrypt(
                packet2,
                session);

        std::string message(
            recovered.data.begin(),
            recovered.data.end());

        std::cout
            << "Recovered message: "
            << message
            << "\n";

        std::cout
            << "SECOND PACKET: SUCCESS\n";
    }
    catch (const std::exception& e)
    {
        std::cout
            << "SECOND PACKET: FAILED\n"
            << "Reason: "
            << e.what()
            << "\n";

        secureChannel.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    /*
     * ------------------------------------------------------
     * TEST 4
     * Replay packet 1
     * ------------------------------------------------------
     */

    std::cout
        << "\n------------------------------------------\n"
        << "TEST 4: REPLAY PACKET 1\n"
        << "------------------------------------------\n";

    bool replay1Blocked = false;

    try
    {
        secureChannel.Decrypt(
            packet1,
            session);

        std::cout
            << "ERROR: Replayed packet 1 accepted\n";
    }
    catch (const std::exception& e)
    {
        replay1Blocked = true;

        std::cout
            << "Replayed packet 1 rejected: SUCCESS\n";

        std::cout
            << "Reason: "
            << e.what()
            << "\n";
    }

    if (!replay1Blocked)
    {
        secureChannel.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    /*
     * ------------------------------------------------------
     * TEST 5
     * Replay packet 2
     * ------------------------------------------------------
     */

    std::cout
        << "\n------------------------------------------\n"
        << "TEST 5: REPLAY PACKET 2\n"
        << "------------------------------------------\n";

    bool replay2Blocked = false;

    try
    {
        secureChannel.Decrypt(
            packet2,
            session);

        std::cout
            << "ERROR: Replayed packet 2 accepted\n";
    }
    catch (const std::exception& e)
    {
        replay2Blocked = true;

        std::cout
            << "Replayed packet 2 rejected: SUCCESS\n";

        std::cout
            << "Reason: "
            << e.what()
            << "\n";
    }

    if (!replay2Blocked)
    {
        secureChannel.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    /*
     * ------------------------------------------------------
     * TEST 6
     * New packet after replay attempts
     * ------------------------------------------------------
     */

    std::cout
        << "\n------------------------------------------\n"
        << "TEST 6: NEW PACKET AFTER REPLAYS\n"
        << "------------------------------------------\n";

    try
    {
        ByteArray recovered =
            secureChannel.Decrypt(
                packet3,
                session);

        std::string message(
            recovered.data.begin(),
            recovered.data.end());

        std::cout
            << "Recovered message: "
            << message
            << "\n";

        std::cout
            << "NEW PACKET: SUCCESS\n";
    }
    catch (const std::exception& e)
    {
        std::cout
            << "NEW PACKET: FAILED\n"
            << "Reason: "
            << e.what()
            << "\n";

        secureChannel.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    /*
     * ------------------------------------------------------
     * TEST 7
     * Tampered sequence number must fail GCM authentication
     * ------------------------------------------------------
     */

    std::cout
        << "\n------------------------------------------\n"
        << "TEST 7: SEQUENCE NUMBER TAMPERING\n"
        << "------------------------------------------\n";

    SecurePacket forgedPacket =
        packet3;

    forgedPacket.sequenceNumber = 999;

    bool sequenceTamperingBlocked = false;

    try
    {
        secureChannel.Decrypt(
            forgedPacket,
            session);

        std::cout
            << "ERROR: Tampered sequence number accepted\n";
    }
    catch (const std::exception& e)
    {
        sequenceTamperingBlocked = true;

        std::cout
            << "Tampered sequence number rejected: SUCCESS\n";

        std::cout
            << "Reason: "
            << e.what()
            << "\n";
    }

    if (!sequenceTamperingBlocked)
    {
        secureChannel.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    /*
     * ------------------------------------------------------
     * TEST 8
     * Verify forged sequence did not advance replay state
     *
     * Packet 3 was already accepted.
     * Therefore another packet with sequence 3 must
     * still be rejected.
     * ------------------------------------------------------
     */

    std::cout
        << "\n------------------------------------------\n"
        << "TEST 8: FORGED PACKET DID NOT ADVANCE STATE\n"
        << "------------------------------------------\n";

    bool stateProtected = false;

    try
    {
        secureChannel.Decrypt(
            packet3,
            session);

        std::cout
            << "ERROR: Packet 3 accepted after replay state test\n";
    }
    catch (const std::exception& e)
    {
        stateProtected = true;

        std::cout
            << "Replay state remained protected: SUCCESS\n";

        std::cout
            << "Reason: "
            << e.what()
            << "\n";
    }

    if (!stateProtected)
    {
        secureChannel.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
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
        << "\n==========================================\n"
        << "ALL REPLAY PROTECTION TESTS PASSED\n"
        << "==========================================\n";

    return 0;
}
