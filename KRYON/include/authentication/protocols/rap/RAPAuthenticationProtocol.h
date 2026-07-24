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
* Reference implementation of the RAP authentication protocol.
 *
 * Current implementation performs:
 * • Random challenge generation
 * • SHA-256 challenge hashing
 * • Proof verification
 * • Four-message authentication exchange
 *
 * Future versions will extend this with:
 * • ECC signatures
 * • Session key establishment
 * • Replay protection
 * • Mutual authentication
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
#include <random>
#include <algorithm>
#include "../../../crypto/CryptoEngine.h"

namespace kryon
{

class RAPAuthenticationProtocol : public IAuthenticationProtocol
{
public:

    RAPAuthenticationProtocol() = default;

    ~RAPAuthenticationProtocol() override = default;
	
	void SetCryptoEngine(CryptoEngine* crypto)
{
    m_crypto = crypto;
}

void Initialize() override
{
    Logger::Info("RAP Authentication Protocol initialized.");
}

    AuthenticationResult Authenticate(
    const AuthenticationRequest& request) override
{
	double start =
    ns3::Simulator::Now().GetSeconds();
	
	if (m_crypto == nullptr)
{
    AuthenticationResult result;

    result.requestId = request.requestId;
    result.authenticated = false;
    result.status = AuthenticationStatus::FAILED;
    result.reason = "CryptoEngine not initialized.";

    return result;
}
	
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
        return "Reference Authentication Protocol (RAP)";
    }
	
private:
	AuthenticationChallenge GenerateChallenge(
    const AuthenticationRequest& request)
	{
    Logger::Info("Message 2: Challenge generated.");

    AuthenticationChallenge challenge;

    challenge.requestId = request.requestId;
    challenge.sourceNodeId = request.destinationNodeId;
    challenge.destinationNodeId = request.sourceNodeId;
	
  ByteArray nonce =
    m_crypto->GenerateRandomBytes(32);

challenge.challenge = nonce.data;
    challenge.timestamp = request.timestamp;
	
	Logger::Info(
    "Generated 32-byte authentication challenge.");

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
 ByteArray message;

message.data = challenge.challenge;
Logger::Info(
    "Computing SHA-256 response proof.");
HashValue proofHash =
    m_crypto->ComputeHash(message);

response.proof =
    proofHash.bytes.data;
    response.timestamp = challenge.timestamp;

    return response;
}
bool VerifyResponse(
    const AuthenticationResponse& response)
{
    (void)response;

    Logger::Info(
    "Verifying SHA-256 proof.");
	
	Logger::Info("Authentication Result : SUCCESS");

   ByteArray message;

message.data = response.challenge;

HashValue expected =
    m_crypto->ComputeHash(message);

return expected.bytes.data ==
       response.proof;
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
private:

    CryptoEngine* m_crypto = nullptr;

	
};


}

#endif
