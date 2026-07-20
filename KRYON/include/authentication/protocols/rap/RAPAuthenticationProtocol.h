#ifndef KRYON_RAP_AUTHENTICATION_PROTOCOL_H
#define KRYON_RAP_AUTHENTICATION_PROTOCOL_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : RAPAuthenticationProtocol.h
 *
 * Description
 * -----------
 * A simple authentication protocol used only for validating
 * the authentication framework architecture.
 *
 * This protocol performs no cryptographic operations and
 * always returns a successful authentication result.
 * ----------------------------------------------------------
 */

#include "../../IAuthenticationProtocol.h"
#include "../../AuthenticationRequest.h"
#include "../../AuthenticationResult.h"
#include "../../AuthenticationTypes.h"
#include "../../../core/Logger.h"
#include "../../AuthenticationChallenge.h"
#include "../../AuthenticationResponse.h"
#include "ns3/core-module.h"

namespace kryon
{

class RAPAuthenticationProtocol : public IAuthenticationProtocol
{
public:

    RAPAuthenticationProtocol() = default;

    ~RAPAuthenticationProtocol() override = default;

    void Initialize() override
    {
        Logger::Info("RAP  Authentication Protocol initialized.");
    }

    AuthenticationResult Authenticate(
    const AuthenticationRequest& request) override
{
	double start =
    ns3::Simulator::Now().GetSeconds();
	
    Logger::Info("==========================================");
Logger::Info("RAP Authentication Started");

Logger::Info(
    "Drone Node      : " +
    std::to_string(request.sourceNodeId));

Logger::Info(
    "Vehicle Node    : " +
    std::to_string(request.destinationNodeId));

Logger::Info(
    "Request ID      : " +
    request.requestId);

Logger::Info("------------------------------------------");
Logger::Info("Message 1 : Authentication Request");

    AuthenticationChallenge challenge =
        GenerateChallenge(request);

    AuthenticationResponse response =
        GenerateResponse(request, challenge);

    bool authenticated =
        VerifyResponse(response);

    Logger::Info("------------------------------------------");
	Logger::Info("RAP Authentication Completed");
	Logger::Info("==========================================");
	
	double end =
    ns3::Simulator::Now().GetSeconds();

    return BuildResult(
    request,
    authenticated,
    (end - start) * 1000.0);
}

    void Finalize() override
    {
        Logger::Info("RAP  Authentication Protocol finalized.");
    }

    std::string GetProtocolName() const override
    {
        return "RReference Authentication Protocol (RAP)";
    }
	
private:
	AuthenticationChallenge GenerateChallenge(
    const AuthenticationRequest& request)
	{
    Logger::Info("Message 2: Challenge generated.");

    AuthenticationChallenge challenge;

    challenge.challengeId = "CH-0001";
    challenge.sourceNodeId = request.destinationNodeId;
    challenge.destinationNodeId = request.sourceNodeId;
    challenge.challenge = 123456;
    challenge.timestamp = request.timestamp;

    return challenge;
	}
	
	AuthenticationResponse GenerateResponse(
    const AuthenticationRequest& request,
    const AuthenticationChallenge& challenge)
{
    Logger::Info("Message  3: Challenge Response.");

    AuthenticationResponse response;

    response.requestId = request.requestId;
    response.responderNodeId = request.sourceNodeId;
    response.challenge = challenge.challenge;
    response.proof = "RAP-PROOF";
    response.timestamp = challenge.timestamp;

    return response;
}
bool VerifyResponse(
    const AuthenticationResponse& response)
{
    (void)response;

    Logger::Info("Message  4: Response Verification.");
	
	Logger::Info("Authentication Result : SUCCESS");

    return true;
}
AuthenticationResult BuildResult(
    const AuthenticationRequest& request,
    bool authenticated,
    double authenticationTimeMs)
{
   AuthenticationResult result;

result.requestId = request.requestId;

result.protocolName = "RAP";

result.method = request.method;

result.status =
    authenticated ?
    AuthenticationStatus::SUCCESS :
    AuthenticationStatus::FAILED;

result.authenticated = authenticated;

/*
 * RAP Message Flow
 * 1. Authentication Request
 * 2. Challenge
 * 3. Challenge Response
 * 4. Verification
 */
result.messagesExchanged = 4;

/*
 * Current RAP communication overhead
 *
 * Request  : 836 Bytes
 * Response : 68 Bytes
 */
result.bytesExchanged = 904;

result.authenticationTimeMs =
    authenticationTimeMs;

result.reason =
    authenticated ?
    "RAP authentication successful." :
    "RAP authentication failed.";

return result;

}
	
};

}

#endif
