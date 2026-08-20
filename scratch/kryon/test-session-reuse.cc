#include "../../KRYON/include/authentication/SessionManager.h"
#include "../../KRYON/include/authentication/Session.h"
#include "../../KRYON/include/crypto/CryptoTypes.h"
#include "../../KRYON/include/core/ExperimentConfig.h"
#include "../../KRYON/include/simulation/SimulationContext.h"

#include <iostream>

int main()
{
    std::cout
        << "=========================================="
        << std::endl;

    std::cout
        << "KRYON SESSION REUSE TEST"
        << std::endl;

    std::cout
        << "=========================================="
        << std::endl;

    kryon::ExperimentConfig config;

    kryon::SimulationContext context;

    kryon::SessionManager sessionManager(
        config,
        context);

    sessionManager.Initialize();

    /*
     * ------------------------------------------------------
     * Create Session
     * ------------------------------------------------------
     */

    kryon::Session session;

    session.sessionId =
        "KRYON-SESSION-1-2-TEST";

    session.droneId = 1;

    session.vehicleId = 2;

    session.creationTime = 10.0;

    session.expirationTime = 610.0;

    session.active = true;

    /*
     * Dummy 32-byte session key
     */

    session.sessionKey.bytes.data.resize(32);

    sessionManager.CreateSession(session);

    std::cout
        << "Session created."
        << std::endl;

    /*
     * ------------------------------------------------------
     * TEST 1: Session should be reusable
     * ------------------------------------------------------
     */

    std::cout
        << std::endl
        << "TEST 1: VALID SESSION"
        << std::endl;

    kryon::Session* existingSession =
        sessionManager.FindSession(
            1,
            2,
            100.0);

    if (existingSession != nullptr)
    {
        std::cout
            << "SESSION REUSE: SUCCESS"
            << std::endl;

        std::cout
            << "Session ID : "
            << existingSession->sessionId
            << std::endl;

        std::cout
            << "Session key size : "
            << existingSession->sessionKey.bytes.data.size()
            << " bytes"
            << std::endl;
    }
    else
    {
        std::cout
            << "SESSION REUSE: FAILED"
            << std::endl;

        return 1;
    }

    /*
     * ------------------------------------------------------
     * TEST 2: Expired session should NOT be reused
     * ------------------------------------------------------
     */

    std::cout
        << std::endl
        << "TEST 2: EXPIRED SESSION"
        << std::endl;

    kryon::Session* expiredSession =
        sessionManager.FindSession(
            1,
            2,
            700.0);

    if (expiredSession == nullptr)
    {
        std::cout
            << "EXPIRED SESSION REUSE BLOCKED: SUCCESS"
            << std::endl;
    }
    else
    {
        std::cout
            << "EXPIRED SESSION REUSE BLOCKED: FAILED"
            << std::endl;

        return 1;
    }

    /*
     * ------------------------------------------------------
     * TEST 3: Wrong vehicle should not reuse session
     * ------------------------------------------------------
     */

    std::cout
        << std::endl
        << "TEST 3: WRONG VEHICLE"
        << std::endl;

    kryon::Session* wrongVehicleSession =
        sessionManager.FindSession(
            1,
            99,
            100.0);

    if (wrongVehicleSession == nullptr)
    {
        std::cout
            << "WRONG PEER SESSION BLOCKED: SUCCESS"
            << std::endl;
    }
    else
    {
        std::cout
            << "WRONG PEER SESSION BLOCKED: FAILED"
            << std::endl;

        return 1;
    }

    /*
     * ------------------------------------------------------
     * TEST 4: Invalidated session should not be reused
     * ------------------------------------------------------
     */

    std::cout
        << std::endl
        << "TEST 4: INVALIDATED SESSION"
        << std::endl;

    sessionManager.InvalidateSession(
        "KRYON-SESSION-1-2-TEST");

    kryon::Session* invalidatedSession =
        sessionManager.FindSession(
            1,
            2,
            100.0);

    if (invalidatedSession == nullptr)
    {
        std::cout
            << "INVALIDATED SESSION BLOCKED: SUCCESS"
            << std::endl;
    }
    else
    {
        std::cout
            << "INVALIDATED SESSION BLOCKED: FAILED"
            << std::endl;

        return 1;
    }

    /*
     * ------------------------------------------------------
     * Finalize
     * ------------------------------------------------------
     */

    sessionManager.Finalize();

    std::cout
        << std::endl
        << "=========================================="
        << std::endl;

    std::cout
        << "SESSION REUSE TEST COMPLETED"
        << std::endl;

    std::cout
        << "=========================================="
        << std::endl;

    return 0;
}
