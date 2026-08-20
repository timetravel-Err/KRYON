#include "../../KRYON/include/authentication/SessionManager.h"
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
        << "KRYON SESSION MANAGER TEST"
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
     * Create dummy session key
     * ------------------------------------------------------
     */

    kryon::SessionKey sessionKey;

    sessionKey.bytes.data =
        std::vector<uint8_t>(
            32,
            0xAA);


    /*
     * ------------------------------------------------------
     * Create authenticated session
     * ------------------------------------------------------
     */

    double currentTime = 10.0;

    double sessionLifetime = 600.0;

    kryon::Session session =
        sessionManager.CreateAuthenticatedSession(
            1,
            2,
            sessionKey,
            currentTime,
            sessionLifetime);


    std::cout
        << "Session ID        : "
        << session.sessionId
        << std::endl;

    std::cout
        << "Drone ID           : "
        << session.droneId
        << std::endl;

    std::cout
        << "Vehicle ID         : "
        << session.vehicleId
        << std::endl;

    std::cout
        << "Session key size   : "
        << session.sessionKey.bytes.data.size()
        << " bytes"
        << std::endl;

    std::cout
        << "Creation time      : "
        << session.creationTime
        << " s"
        << std::endl;

    std::cout
        << "Expiration time    : "
        << session.expirationTime
        << " s"
        << std::endl;

    std::cout
        << "Active             : "
        << (session.active ? "YES" : "NO")
        << std::endl;


    /*
     * ------------------------------------------------------
     * Find session
     * ------------------------------------------------------
     */

    kryon::Session* found =
        sessionManager.FindSession(
            1,
            2,
            100.0);


    if (found != nullptr)
    {
        std::cout
            << "SESSION FIND: SUCCESS"
            << std::endl;
    }
    else
    {
        std::cout
            << "SESSION FIND: FAILED"
            << std::endl;

        return 1;
    }


    /*
     * ------------------------------------------------------
     * Test expiration
     * ------------------------------------------------------
     */

    kryon::Session* expired =
        sessionManager.FindSession(
            1,
            2,
            700.0);


    if (expired == nullptr)
    {
        std::cout
            << "SESSION EXPIRATION: SUCCESS"
            << std::endl;
    }
    else
    {
        std::cout
            << "SESSION EXPIRATION: FAILED"
            << std::endl;

        return 1;
    }


    sessionManager.RemoveExpiredSessions(
        700.0);


    sessionManager.Finalize();


    std::cout
        << "=========================================="
        << std::endl;

    std::cout
        << "SESSION MANAGER TEST COMPLETED"
        << std::endl;

    std::cout
        << "=========================================="
        << std::endl;


    return 0;
}
