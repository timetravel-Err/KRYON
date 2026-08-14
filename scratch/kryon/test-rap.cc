#include "../../KRYON/include/authentication/protocols/rap/RAPAuthenticationProtocol.h"
#include "../../KRYON/include/authentication/AuthenticationRequest.h"
#include "../../KRYON/include/crypto/CryptoEngine.h"
#include "../../KRYON/include/core/ExperimentConfig.h"
#include "../../KRYON/include/simulation/SimulationContext.h"
#include "../../KRYON/include/core/Logger.h"

#include <iostream>

int main()
{
    kryon::Logger::Info(
        "==========================================");

    kryon::Logger::Info(
        "KRYON RAP AUTHENTICATION TEST");

    kryon::Logger::Info(
        "==========================================");

    /*
     * ------------------------------------------------------
     * Configuration and simulation context
     * ------------------------------------------------------
     */

    kryon::ExperimentConfig config;
    kryon::SimulationContext context;

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

    rap.Initialize();

    /*
     * ------------------------------------------------------
     * Authentication Request
     * ------------------------------------------------------
     */

    kryon::AuthenticationRequest request;

    request.requestId =
        "RAP-TEST-001";

    request.sourceNodeId =
        1;

    request.destinationNodeId =
        2;

    request.timestamp =
        1.0;

    request.method =
        kryon::AuthenticationMethod::ECC;

    /*
     * ------------------------------------------------------
     * Execute RAP authentication
     * ------------------------------------------------------
     */

    kryon::AuthenticationResult result =
        rap.Authenticate(request);

    /*
     * ------------------------------------------------------
     * Display result
     * ------------------------------------------------------
     */

    std::cout
        << "\n------------------------------------------"
        << std::endl;

    std::cout
        << "RAP RESULT"
        << std::endl;

    std::cout
        << "------------------------------------------"
        << std::endl;

    std::cout
        << "Request ID        : "
        << result.requestId
        << std::endl;

    std::cout
        << "Protocol          : "
        << result.protocolName
        << std::endl;

    std::cout
        << "Authenticated     : "
        << (result.authenticated ? "YES" : "NO")
        << std::endl;

    std::cout
        << "Messages exchanged: "
        << result.messagesExchanged
        << std::endl;

    std::cout
        << "Bytes exchanged   : "
        << result.bytesExchanged
        << std::endl;

    std::cout
        << "Authentication time: "
        << result.authenticationTimeMs
        << " ms"
        << std::endl;

    std::cout
        << "Session ID        : "
        << result.sessionId
        << std::endl;

    std::cout
        << "Session key size  : "
        << result.sessionKey.bytes.data.size()
        << " bytes"
        << std::endl;

    std::cout
        << "Reason            : "
        << result.reason
        << std::endl;

    /*
     * ------------------------------------------------------
     * Determine test result
     * ------------------------------------------------------
     */

    if (!result.authenticated)
    {
        std::cout
            << "\nRAP AUTHENTICATION: FAILED"
            << std::endl;

        rap.Finalize();
        crypto.Finalize();

        return 1;
    }

    if (result.sessionKey.bytes.data.empty())
    {
        std::cout
            << "\nSESSION KEY: FAILED"
            << std::endl;

        rap.Finalize();
        crypto.Finalize();

        return 1;
    }

    std::cout
        << "\nRAP AUTHENTICATION: SUCCESS"
        << std::endl;

    std::cout
        << "SESSION KEY: SUCCESS"
        << std::endl;

    /*
     * ------------------------------------------------------
     * Finalize
     * ------------------------------------------------------
     */

    rap.Finalize();
    crypto.Finalize();

    std::cout
        << "\n=========================================="
        << std::endl;

    std::cout
        << "RAP TEST COMPLETED SUCCESSFULLY"
        << std::endl;

    std::cout
        << "=========================================="
        << std::endl;

    return 0;
}
