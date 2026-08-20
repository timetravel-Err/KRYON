#include "ns3/core-module.h"

#include "../../KRYON/include/core/ExperimentConfig.h"
#include "../../KRYON/include/simulation/SimulationContext.h"
#include "../../KRYON/include/crypto/CryptoEngine.h"
#include "../../KRYON/include/security/SecureChannel.h"
#include "../../KRYON/include/authentication/Session.h"
#include "../../KRYON/include/authentication/SessionManager.h"

#include <iostream>
#include <string>
#include <stdexcept>

using namespace ns3;
using namespace kryon;

int main(int argc, char* argv[])
{
    CommandLine cmd;
    cmd.Parse(argc, argv);

    std::cout << "==========================================\n";
    std::cout << "KRYON SECURE CHANNEL TEST\n";
    std::cout << "==========================================\n";

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
	secureChannel.SetSessionManager(
    &sessionManager);
    secureChannel.Initialize();

    /*
     * ------------------------------------------------------
     * Create authenticated test session
     * ------------------------------------------------------
     */

    Session session;

    session.sessionId =
        "KRYON-SESSION-1-2-SECURE-TEST";

    session.droneId = 1;
    session.vehicleId = 2;

    session.creationTime =
        Simulator::Now().GetSeconds();

    session.expirationTime =
        session.creationTime + 600.0;

    session.active = true;

    /*
     * Generate a 32-byte session key.
     *
     * In the real protocol this comes from ECDH + HKDF.
     * Here we generate it directly because the purpose
     * of this test is the SecureChannel.
     */

    session.sessionKey.bytes =
        crypto.GenerateRandomBytes(32);
		
	/*
	 * Register the authenticated session with
	 * SessionManager so SecureChannel can perform
	 * sequence-number management.
	 */	
			
	sessionManager.CreateSession(session);	

    std::cout << "\n------------------------------------------\n";
    std::cout << "SESSION SETUP\n";
    std::cout << "------------------------------------------\n";

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
     * Normal encryption/decryption
     * ------------------------------------------------------
     */

    std::cout << "\n------------------------------------------\n";
    std::cout << "TEST 1: AES-256-GCM ENCRYPTION\n";
    std::cout << "------------------------------------------\n";

    std::string message =
        "Hello Secure KRYON";

    ByteArray plaintext;

    plaintext.data.assign(
        message.begin(),
        message.end());

    SecurePacket packet =
        secureChannel.Encrypt(
            plaintext,
            session);

    std::cout
        << "Plaintext size : "
        << plaintext.data.size()
        << " bytes\n";

    std::cout
        << "Ciphertext size: "
        << packet.ciphertext.data.size()
        << " bytes\n";

    std::cout
        << "Nonce size     : "
        << packet.nonce.data.size()
        << " bytes\n";

    std::cout
        << "GCM tag size   : "
        << packet.mac.data.size()
        << " bytes\n";

    std::cout
        << "Encrypted flag : "
        << (packet.encrypted ? "YES" : "NO")
        << "\n";

    if (packet.ciphertext.data == plaintext.data)
    {
        std::cout
            << "WARNING: ciphertext equals plaintext\n";
    }

    else
    {
        std::cout
            << "Ciphertext differs from plaintext: SUCCESS\n";
    }

    /*
     * ------------------------------------------------------
     * TEST 2
     * Normal decryption
     * ------------------------------------------------------
     */

    std::cout << "\n------------------------------------------\n";
    std::cout << "TEST 2: AES-256-GCM DECRYPTION\n";
    std::cout << "------------------------------------------\n";

    ByteArray recovered =
        secureChannel.Decrypt(
            packet,
            session);

    std::string recoveredMessage(
        recovered.data.begin(),
        recovered.data.end());

    std::cout
        << "Recovered message: "
        << recoveredMessage
        << "\n";

    if (recovered.data == plaintext.data)
    {
        std::cout
            << "DECRYPTION: SUCCESS\n";
    }
    else
    {
        std::cout
            << "DECRYPTION: FAILED\n";

        crypto.Finalize();
        return 1;
    }

    /*
     * ------------------------------------------------------
     * TEST 3
     * Ciphertext tampering
     * ------------------------------------------------------
     */

    std::cout << "\n------------------------------------------\n";
    std::cout << "TEST 3: CIPHERTEXT TAMPERING\n";
    std::cout << "------------------------------------------\n";

    SecurePacket tamperedCiphertext =
        packet;

    if (!tamperedCiphertext.ciphertext.data.empty())
    {
        tamperedCiphertext.ciphertext.data[0] ^=
            0x01;
    }

    bool tamperingBlocked = false;

    try
    {
        secureChannel.Decrypt(
            tamperedCiphertext,
            session);

        std::cout
            << "ERROR: Tampered ciphertext accepted\n";
    }
    catch (const std::exception& e)
    {
        tamperingBlocked = true;

        std::cout
            << "Tampered ciphertext rejected: SUCCESS\n";

        std::cout
            << "Reason: "
            << e.what()
            << "\n";
    }

    if (!tamperingBlocked)
    {
        crypto.Finalize();
        return 1;
    }

    /*
     * ------------------------------------------------------
     * TEST 4
     * Authentication tag tampering
     * ------------------------------------------------------
     */

    std::cout << "\n------------------------------------------\n";
    std::cout << "TEST 4: GCM TAG TAMPERING\n";
    std::cout << "------------------------------------------\n";

    SecurePacket tamperedTag =
        packet;

    if (!tamperedTag.mac.data.empty())
    {
        tamperedTag.mac.data[0] ^=
            0x01;
    }

    bool tagTamperingBlocked = false;

    try
    {
        secureChannel.Decrypt(
            tamperedTag,
            session);

        std::cout
            << "ERROR: Tampered GCM tag accepted\n";
    }
    catch (const std::exception& e)
    {
        tagTamperingBlocked = true;

        std::cout
            << "Tampered GCM tag rejected: SUCCESS\n";

        std::cout
            << "Reason: "
            << e.what()
            << "\n";
    }

    if (!tagTamperingBlocked)
    {
        crypto.Finalize();
        return 1;
    }

    /*
     * ------------------------------------------------------
     * TEST 5
     * Wrong session key
     * ------------------------------------------------------
     */

    std::cout << "\n------------------------------------------\n";
    std::cout << "TEST 5: WRONG SESSION KEY\n";
    std::cout << "------------------------------------------\n";

    Session wrongKeySession =
        session;

    wrongKeySession.sessionKey.bytes =
        crypto.GenerateRandomBytes(32);

    bool wrongKeyBlocked = false;

    try
    {
        secureChannel.Decrypt(
            packet,
            wrongKeySession);

        std::cout
            << "ERROR: Wrong session key accepted\n";
    }
    catch (const std::exception& e)
    {
        wrongKeyBlocked = true;

        std::cout
            << "Wrong session key rejected: SUCCESS\n";

        std::cout
            << "Reason: "
            << e.what()
            << "\n";
    }

    if (!wrongKeyBlocked)
    {
        crypto.Finalize();
        return 1;
    }

    /*
     * ------------------------------------------------------
     * TEST 6
     * Wrong session ID
     * ------------------------------------------------------
     */

    std::cout << "\n------------------------------------------\n";
    std::cout << "TEST 6: WRONG SESSION ID\n";
    std::cout << "------------------------------------------\n";

    SecurePacket wrongSessionPacket =
        packet;

    wrongSessionPacket.sessionId =
        "KRYON-SESSION-ATTACKER";

    bool wrongSessionBlocked = false;

    try
    {
        secureChannel.Decrypt(
            wrongSessionPacket,
            session);

        std::cout
            << "ERROR: Wrong session ID accepted\n";
    }
    catch (const std::exception& e)
    {
        wrongSessionBlocked = true;

        std::cout
            << "Wrong session ID rejected: SUCCESS\n";

        std::cout
            << "Reason: "
            << e.what()
            << "\n";
    }

    if (!wrongSessionBlocked)
    {
        crypto.Finalize();
        return 1;
    }

    /*
     * ------------------------------------------------------
     * TEST 7
     * Inactive session
     * ------------------------------------------------------
     */

    std::cout << "\n------------------------------------------\n";
    std::cout << "TEST 7: INACTIVE SESSION\n";
    std::cout << "------------------------------------------\n";

    Session inactiveSession =
        session;

    inactiveSession.active =
        false;

    bool inactiveBlocked = false;

    try
    {
        secureChannel.Decrypt(
            packet,
            inactiveSession);

        std::cout
            << "ERROR: Inactive session accepted\n";
    }
    catch (const std::exception& e)
    {
        inactiveBlocked = true;

        std::cout
            << "Inactive session rejected: SUCCESS\n";

        std::cout
            << "Reason: "
            << e.what()
            << "\n";
    }

    if (!inactiveBlocked)
    {
        crypto.Finalize();
        return 1;
    }

    /*
     * ------------------------------------------------------
     * Final result
     * ------------------------------------------------------
     */

    secureChannel.Finalize();
	sessionManager.Finalize();
    crypto.Finalize();

    std::cout << "\n==========================================\n";
    std::cout << "ALL SECURE CHANNEL TESTS PASSED\n";
    std::cout << "==========================================\n";

    return 0;
}
