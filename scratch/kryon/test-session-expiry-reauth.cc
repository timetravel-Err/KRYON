#include "../../KRYON/include/authentication/protocols/rap/RAPAuthenticationProtocol.h"
#include "../../KRYON/include/authentication/AuthenticationRequest.h"
#include "../../KRYON/include/crypto/CryptoEngine.h"
#include "../../KRYON/include/core/ExperimentConfig.h"
#include "../../KRYON/include/simulation/SimulationContext.h"
#include "../../KRYON/include/core/Logger.h"
#include "../../KRYON/include/authentication/SessionManager.h"

#include <iostream>

int main()
{
    std::cout
        << "=========================================="
        << std::endl;

    std::cout
        << "KRYON SESSION EXPIRY + REAUTH TEST"
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
     * RAP Protocol
     * ------------------------------------------------------
     */

    kryon::RAPAuthenticationProtocol rap;

    rap.SetCryptoEngine(&crypto);
    rap.SetSessionManager(&sessionManager);

    rap.Initialize();

    /*
     * ------------------------------------------------------
     * FIRST AUTHENTICATION
     * ------------------------------------------------------
     */

    std::cout
        << "\n=========================================="
        << std::endl;

    std::cout
        << "TEST 1: INITIAL AUTHENTICATION"
        << std::endl;

    std::cout
        << "=========================================="
        << std::endl;

    kryon::AuthenticationRequest request1;

    request1.requestId =
        "RAP-EXPIRY-001";

    request1.sourceNodeId =
        1;

    request1.destinationNodeId =
        2;

    request1.timestamp =
        1.0;

    request1.method =
        kryon::AuthenticationMethod::ECC;

    kryon::AuthenticationResult result1 =
        rap.Authenticate(request1);

    std::cout
        << "\nAuthenticated : "
        << (result1.authenticated ? "YES" : "NO")
        << std::endl;

    std::cout
        << "Session ID    : "
        << result1.sessionId
        << std::endl;

    std::cout
        << "Session key   : "
        << result1.sessionKey.bytes.data.size()
        << " bytes"
        << std::endl;

    if (!result1.authenticated)
    {
        std::cout
            << "\nINITIAL AUTHENTICATION: FAILED"
            << std::endl;

        rap.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    if (result1.sessionKey.bytes.data.size() != 32)
    {
        std::cout
            << "\nSESSION KEY SIZE: FAILED"
            << std::endl;

        rap.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    const std::string originalSessionId =
        result1.sessionId;

    /*
     * ------------------------------------------------------
     * SESSION REUSE BEFORE EXPIRATION
     * ------------------------------------------------------
     */

    std::cout
        << "\n=========================================="
        << std::endl;

    std::cout
        << "TEST 2: VALID SESSION REUSE"
        << std::endl;

    std::cout
        << "=========================================="
        << std::endl;

    kryon::AuthenticationRequest request2;

    request2.requestId =
        "RAP-EXPIRY-002";

    request2.sourceNodeId =
        1;

    request2.destinationNodeId =
        2;

    request2.timestamp =
        2.0;

    request2.method =
        kryon::AuthenticationMethod::ECC;

    kryon::AuthenticationResult result2 =
        rap.Authenticate(request2);

    std::cout
        << "\nAuthenticated : "
        << (result2.authenticated ? "YES" : "NO")
        << std::endl;

    std::cout
        << "Session ID    : "
        << result2.sessionId
        << std::endl;

    std::cout
        << "Messages      : "
        << result2.messagesExchanged
        << std::endl;

    std::cout
        << "Bytes         : "
        << result2.bytesExchanged
        << std::endl;

    if (!result2.authenticated ||
        result2.sessionId != originalSessionId ||
        result2.messagesExchanged != 0 ||
        result2.bytesExchanged != 0)
    {
        std::cout
            << "\nVALID SESSION REUSE: FAILED"
            << std::endl;

        rap.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    std::cout
        << "\nVALID SESSION REUSE: SUCCESS"
        << std::endl;

    /*
     * ------------------------------------------------------
     * EXPIRE THE SESSION
     * ------------------------------------------------------
     *
     * Session lifetime is currently 600 seconds.
     *
     * We use a time beyond that lifetime.
     * ------------------------------------------------------
     */

    std::cout
        << "\n=========================================="
        << std::endl;

    std::cout
        << "TEST 3: SESSION EXPIRATION"
        << std::endl;

    std::cout
        << "=========================================="
        << std::endl;

    const double expirationTestTime =
        1000.0;

    sessionManager.RemoveExpiredSessions(
        expirationTestTime);

    /*
     * ------------------------------------------------------
     * VERIFY SESSION CANNOT BE REUSED
     * ------------------------------------------------------
     */

    kryon::Session* expiredSession =
        sessionManager.FindSession(
            1,
            2,
            expirationTestTime);

    if (expiredSession != nullptr)
    {
        std::cout
            << "\nSESSION EXPIRATION: FAILED"
            << std::endl;

        rap.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    std::cout
        << "\nSESSION EXPIRATION: SUCCESS"
        << std::endl;

    /*
     * ------------------------------------------------------
     * FULL RE-AUTHENTICATION
     * ------------------------------------------------------
     */

    std::cout
        << "\n=========================================="
        << std::endl;

    std::cout
        << "TEST 4: RE-AUTHENTICATION AFTER EXPIRY"
        << std::endl;

    std::cout
        << "=========================================="
        << std::endl;

    kryon::AuthenticationRequest request3;

    request3.requestId =
        "RAP-EXPIRY-003";

    request3.sourceNodeId =
        1;

    request3.destinationNodeId =
        2;

    request3.timestamp =
        expirationTestTime;

    request3.method =
        kryon::AuthenticationMethod::ECC;

    kryon::AuthenticationResult result3 =
        rap.Authenticate(request3);

    std::cout
        << "\nAuthenticated : "
        << (result3.authenticated ? "YES" : "NO")
        << std::endl;

    std::cout
        << "New Session ID : "
        << result3.sessionId
        << std::endl;

    std::cout
        << "Session key    : "
        << result3.sessionKey.bytes.data.size()
        << " bytes"
        << std::endl;

    std::cout
        << "Messages       : "
        << result3.messagesExchanged
        << std::endl;

    std::cout
        << "Bytes          : "
        << result3.bytesExchanged
        << std::endl;

    /*
     * ------------------------------------------------------
     * Verify re-authentication
     * ------------------------------------------------------
     */

    if (!result3.authenticated)
    {
        std::cout
            << "\nRE-AUTHENTICATION: FAILED"
            << std::endl;

        rap.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    if (result3.sessionKey.bytes.data.size() != 32)
    {
        std::cout
            << "\nNEW SESSION KEY: FAILED"
            << std::endl;

        rap.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    if (result3.messagesExchanged == 0)
    {
        std::cout
            << "\nRE-AUTHENTICATION: FAILED"
            << std::endl;

        std::cout
            << "Expected full RAP authentication after expiry."
            << std::endl;

        rap.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    if (result3.bytesExchanged == 0)
    {
        std::cout
            << "\nRE-AUTHENTICATION: FAILED"
            << std::endl;

        std::cout
            << "Expected RAP communication after expiry."
            << std::endl;

        rap.Finalize();
        sessionManager.Finalize();
        crypto.Finalize();

        return 1;
    }

    /*
     * ------------------------------------------------------
     * Final result
     * ------------------------------------------------------
     */

    std::cout
        << "\nRE-AUTHENTICATION AFTER EXPIRY: SUCCESS"
        << std::endl;

    /*
     * ------------------------------------------------------
     * Finalize
     * ------------------------------------------------------
     */

    rap.Finalize();
    sessionManager.Finalize();
    crypto.Finalize();

    std::cout
        << "\n=========================================="
        << std::endl;

    std::cout
        << "SESSION EXPIRY + REAUTH TEST COMPLETED"
        << std::endl;

    std::cout
        << "=========================================="
        << std::endl;

    return 0;
}
