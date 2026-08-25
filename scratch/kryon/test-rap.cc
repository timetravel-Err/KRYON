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
			 * SECOND AUTHENTICATION
			 * ------------------------------------------------------
			 *
			 * Same Drone and Vehicle.
			 * A valid session should already exist.
			 *
			 * Therefore RAP should reuse the existing session
			 * instead of performing full authentication again.
			 */

			std::cout
				<< "\n=========================================="
				<< std::endl;

			std::cout
				<< "TEST 2: SESSION REUSE"
				<< std::endl;

			std::cout
				<< "=========================================="
				<< std::endl;

			kryon::AuthenticationRequest reuseRequest;

			reuseRequest.requestId =
				"RAP-TEST-002";

			reuseRequest.sourceNodeId =
				1;

			reuseRequest.destinationNodeId =
				2;

			reuseRequest.timestamp =
				2.0;

			reuseRequest.method =
				kryon::AuthenticationMethod::ECC;

			kryon::AuthenticationResult reuseResult =
				rap.Authenticate(reuseRequest);

			std::cout
				<< "\n------------------------------------------"
				<< std::endl;

			std::cout
				<< "SESSION REUSE RESULT"
				<< std::endl;

			std::cout
				<< "------------------------------------------"
				<< std::endl;

			std::cout
				<< "Request ID        : "
				<< reuseResult.requestId
				<< std::endl;

			std::cout
				<< "Authenticated     : "
				<< (reuseResult.authenticated ? "YES" : "NO")
				<< std::endl;

			std::cout
				<< "Session ID        : "
				<< reuseResult.sessionId
				<< std::endl;

			std::cout
				<< "Session key size  : "
				<< reuseResult.sessionKey.bytes.data.size()
				<< " bytes"
				<< std::endl;

			std::cout
				<< "Messages exchanged: "
				<< reuseResult.messagesExchanged
				<< std::endl;

			std::cout
				<< "Bytes exchanged   : "
				<< reuseResult.bytesExchanged
				<< std::endl;

			std::cout
				<< "Authentication time: "
				<< reuseResult.authenticationTimeMs
				<< " ms"
				<< std::endl;

			std::cout
				<< "Reason            : "
				<< reuseResult.reason
				<< std::endl;

			/*
			 * ------------------------------------------------------
			 * Verify session reuse
			 * ------------------------------------------------------
			 */

			if (!reuseResult.authenticated)
			{
				std::cout
					<< "\nSESSION REUSE: FAILED"
					<< std::endl;

				rap.Finalize();
				sessionManager.Finalize();
				crypto.Finalize();

				return 1;
			}

			if (reuseResult.messagesExchanged != 0)
			{
				std::cout
					<< "\nSESSION REUSE: FAILED"
					<< std::endl;

				std::cout
					<< "Expected 0 messages during session reuse."
					<< std::endl;

				rap.Finalize();
				sessionManager.Finalize();
				crypto.Finalize();

				return 1;
			}

			if (reuseResult.bytesExchanged != 0)
			{
				std::cout
					<< "\nSESSION REUSE: FAILED"
					<< std::endl;

				std::cout
					<< "Expected 0 bytes during session reuse."
					<< std::endl;

				rap.Finalize();
				sessionManager.Finalize();
				crypto.Finalize();

				return 1;
			}

			if (reuseResult.sessionId != result.sessionId)
			{
				std::cout
					<< "\nSESSION REUSE: FAILED"
					<< std::endl;

				std::cout
					<< "Session ID changed during reuse."
					<< std::endl;

				rap.Finalize();
				sessionManager.Finalize();
				crypto.Finalize();

				return 1;
			}

			if (reuseResult.sessionKey.bytes.data !=
				result.sessionKey.bytes.data)
			{
				std::cout
					<< "\nSESSION REUSE: FAILED"
					<< std::endl;

				std::cout
					<< "Session key changed during reuse."
					<< std::endl;

				rap.Finalize();
				sessionManager.Finalize();
				crypto.Finalize();

				return 1;
			}

			std::cout
				<< "\nSESSION REUSE: SUCCESS"
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
        << "RAP TEST COMPLETED SUCCESSFULLY"
        << std::endl;

    std::cout
        << "=========================================="
        << std::endl;

    return 0;
}
