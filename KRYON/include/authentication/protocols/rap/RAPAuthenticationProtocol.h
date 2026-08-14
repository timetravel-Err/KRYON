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
#include <chrono>
#include <algorithm>
#include "../../../crypto/CryptoEngine.h"
#include "../../../crypto/CryptoTypes.h"
#include "../../packets/AuthRequestPacket.h"
#include "../../packets/AuthChallengePacket.h"
#include "../../packets/AuthResponsePacket.h"
#include "../../packets/AuthConfirmPacket.h"

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

    Logger::Info(
        "RAP received CryptoEngine pointer " );
}

void Initialize() override
{
    Logger::Info(
        "RAP Authentication Protocol initialized.");

    if (m_crypto == nullptr)
    {
        Logger::Warning(
            "[RAP] CryptoEngine not available during initialization.");
        return;
    }

    Logger::Info(
        "[RAP] Generating Drone ECC key pair.");

    m_droneKeys =
        m_crypto->GenerateKeyPair();

    Logger::Info(
        "[RAP] Generating Vehicle ECC key pair.");

    m_vehicleKeys =
        m_crypto->GenerateKeyPair();
}


AuthenticationResult Authenticate(
    const AuthenticationRequest& request) override
{
	
	//Logger::Info( "RAP Crypto pointer = " + std::to_string(reinterpret_cast<uintptr_t>(m_crypto)));
	
	//double start = ns3::Simulator::Now().GetSeconds();
	auto start =
    std::chrono::steady_clock::now();
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

Logger::Info(
    "Generating Drone ECC key pair.");

m_droneKeys =
    m_crypto->GenerateKeyPair();

Logger::Info(
    "Generating Vehicle ECC key pair.");

m_vehicleKeys =
    m_crypto->GenerateKeyPair();

    AuthenticationChallenge challenge =
        GenerateChallenge(request);

    AuthenticationResponse response =
        GenerateResponse(request, challenge);
		
	VerifyResponse(response);	
		
	Logger::Info(
    "Vehicle deriving ECDH shared secret.");

m_vehicleSecret =
    m_crypto->DeriveSharedSecret(
        m_vehicleKeys.privateKey,
        m_droneKeys.publicKey);

Logger::Info(
    "Drone deriving ECDH shared secret.");

m_droneSecret =
    m_crypto->DeriveSharedSecret(
        m_droneKeys.privateKey,
        m_vehicleKeys.publicKey);	
   
    Logger::Info(
    "Vehicle deriving HKDF session key.");

m_vehicleSessionKey =
    m_crypto->DeriveSessionKey(
        m_vehicleSecret);

Logger::Info(
    "Drone deriving HKDF session key.");

m_droneSessionKey =
    m_crypto->DeriveSessionKey(
        m_droneSecret);
    
	
	bool sharedSecretMatch =
    (m_droneSecret.bytes.data ==
     m_vehicleSecret.bytes.data);

if (sharedSecretMatch)
{
    Logger::Info(
        "ECDH shared secrets match.");
}
else
{
    Logger::Info(
        "ECDH shared secrets DO NOT match.");
}

bool sessionKeyMatch =
    (m_droneSessionKey.bytes.data ==
     m_vehicleSessionKey.bytes.data);

if (sessionKeyMatch)
{
    Logger::Info(
        "HKDF session keys match.");

    Logger::Info(
        "Session Key Length : " +
        std::to_string(
            m_droneSessionKey.bytes.data.size()) +
        " bytes");
}
else
{
    Logger::Info(
        "HKDF session keys DO NOT match.");
}

/*
 * Final authentication decision.
 */
bool authenticated =
    m_vehicleAuthenticated &&
    m_droneAuthenticated &&
    m_proofVerified &&
    sharedSecretMatch &&
    sessionKeyMatch;

if (authenticated)
{
    Logger::Info(
        "Mutual authentication established.");
}
else
{
    Logger::Info(
        "Mutual authentication FAILED.");
}
    Logger::Info("------------------------------------------");
	Logger::Info("RAP Authentication Completed");
	Logger::Info("==========================================");
	
	/*double end =
    ns3::Simulator::Now().GetSeconds();

    return BuildResult(
    request,
    authenticated,
    (end - start) * 1000.0);*/
	
	auto end =
    std::chrono::steady_clock::now();

	double authenticationTimeMs =
		std::chrono::duration<double, std::milli>(
			end - start).count();

	return BuildResult(
		request,
		authenticated,
		authenticationTimeMs);
}



    void Finalize() override
    {
        Logger::Info("RAP  Authentication Protocol finalized.");
    }

       std::string GetProtocolName() const override
    {
        return "Reference Authentication Protocol (RAP)";
    }


    AuthChallengePacket ProcessRequest(
        const AuthRequestPacket& packet) override
    {
        Logger::Info(
            "[RAP] Processing AUTH_REQUEST : " +
            packet.requestId);

        AuthenticationRequest request;

        request.requestId =
            packet.requestId;

        request.sourceNodeId =
            packet.sourceNode;

        request.destinationNodeId =
            packet.destinationNode;

        request.timestamp =
            packet.timestamp;

        AuthenticationChallenge challenge =
            GenerateChallenge(request);

        AuthChallengePacket challengePacket;

        challengePacket.requestId =
            challenge.requestId;

        challengePacket.sourceNode =
            challenge.sourceNodeId;

        challengePacket.destinationNode =
            challenge.destinationNodeId;

        challengePacket.timestamp =
            challenge.timestamp;

        challengePacket.challenge =
            challenge.challenge;

        challengePacket.senderPublicKey =
            challenge.senderPublicKey.bytes.data;

        challengePacket.signature =
            challenge.signature.bytes.data;

        return challengePacket;
    }


    AuthResponsePacket ProcessChallenge(
        const AuthChallengePacket& packet) override
    {
        Logger::Info(
            "[RAP] Processing AUTH_CHALLENGE : " +
            packet.requestId);

        AuthenticationRequest request;

        request.requestId =
            packet.requestId;

        request.sourceNodeId =
            packet.destinationNode;

        request.destinationNodeId =
            packet.sourceNode;

        request.timestamp =
            packet.timestamp;

        AuthenticationChallenge challenge;

        challenge.requestId =
            packet.requestId;

        challenge.sourceNodeId =
            packet.sourceNode;

        challenge.destinationNodeId =
            packet.destinationNode;

        challenge.timestamp =
            packet.timestamp;

        challenge.challenge =
            packet.challenge;

        challenge.senderPublicKey.bytes.data =
            packet.senderPublicKey;

        challenge.signature.bytes.data =
            packet.signature;

        AuthenticationResponse response =
            GenerateResponse(
                request,
                challenge);

        AuthResponsePacket responsePacket;

        responsePacket.requestId =
            response.requestId;

        responsePacket.sourceNode =
            request.sourceNodeId;

        responsePacket.destinationNode =
            request.destinationNodeId;

        responsePacket.timestamp =
            response.timestamp;

        responsePacket.challenge =
            response.challenge;

        responsePacket.proof =
            response.proof;

        responsePacket.senderPublicKey =
            response.senderPublicKey.bytes.data;

        responsePacket.signature =
            response.signature.bytes.data;

        return responsePacket;
    }


    AuthConfirmPacket ProcessResponse(
        const AuthResponsePacket& packet) override
    {
        Logger::Info(
            "[RAP] Processing AUTH_RESPONSE : " +
            packet.requestId);

        AuthenticationResponse response;

        response.requestId =
            packet.requestId;

        response.responderNodeId =
            packet.sourceNode;

        response.timestamp =
            packet.timestamp;

        response.challenge =
            packet.challenge;

        response.proof =
            packet.proof;

        response.senderPublicKey.bytes.data =
            packet.senderPublicKey;

        response.signature.bytes.data =
            packet.signature;

        bool verified =
            VerifyResponse(response);

        AuthConfirmPacket confirmPacket;

        confirmPacket.requestId =
            packet.requestId;

        confirmPacket.sourceNode =
            packet.destinationNode;

        confirmPacket.destinationNode =
            packet.sourceNode;

        confirmPacket.timestamp =
            packet.timestamp;

        confirmPacket.authenticationSuccessful =
            verified;

        confirmPacket.message =
            verified ?
            "RAP authentication successful." :
            "RAP authentication failed.";

        return confirmPacket;
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

/*
 * Attach Vehicle public key.
 */
challenge.senderPublicKey =
    m_vehicleKeys.publicKey;

/*
 * Sign challenge using Vehicle private key.
 */
ByteArray challengeBytes;
challengeBytes.data = challenge.challenge;

challenge.signature =
    m_crypto->Sign(
        challengeBytes,
        m_vehicleKeys.privateKey);

Logger::Info(
    "Vehicle signed authentication challenge.");



    return challenge;
	}
	
	
/*GenerateResponse*/
	
AuthenticationResponse GenerateResponse(
    const AuthenticationRequest& request,
    const AuthenticationChallenge& challenge)
{
    Logger::Info("Message  3: Challenge Response.");

    AuthenticationResponse response;

    response.requestId = request.requestId;
    response.responderNodeId = request.sourceNodeId;
    response.challenge = challenge.challenge;
	
	Logger::Info(
    "Verifying Vehicle signature.");

ByteArray challengeBytes;
challengeBytes.data = challenge.challenge;

m_vehicleAuthenticated =
    m_crypto->Verify(
        challengeBytes,
        challenge.signature,
        challenge.senderPublicKey);

if (!m_vehicleAuthenticated)
{
    Logger::Info(
        "Vehicle signature verification FAILED.");
}
else
{
    Logger::Info(
        "Vehicle successfully authenticated.");
}
 ByteArray message;

message.data = challenge.challenge;
Logger::Info(
    "Computing SHA-256 response proof.");
HashValue proofHash =
    m_crypto->ComputeHash(message);

response.proof =
    proofHash.bytes.data;
	
	/*
 * Attach Drone public key.
 */
response.senderPublicKey =
    m_droneKeys.publicKey;

/*
 * Drone signs the challenge response.
 */
response.signature =
    m_crypto->Sign(
        challengeBytes,
        m_droneKeys.privateKey);

Logger::Info(
    "Drone signed authentication response.");
    response.timestamp = challenge.timestamp;

    return response;
}


bool VerifyResponse(
    const AuthenticationResponse& response)
{
    Logger::Info(
        "Verifying SHA-256 proof.");

    ByteArray message;
    message.data = response.challenge;

    Logger::Info(
        "Verifying Drone signature.");

    m_droneAuthenticated =
        m_crypto->Verify(
            message,
            response.signature,
            response.senderPublicKey);

    if (!m_droneAuthenticated)
    {
        Logger::Info(
            "Drone signature verification FAILED.");

        return false;
    }

    Logger::Info(
        "Drone successfully authenticated.");

    HashValue expected =
        m_crypto->ComputeHash(message);

    m_proofVerified =
        (expected.bytes.data == response.proof);

    if (m_proofVerified)
    {
        Logger::Info(
            "Challenge proof verified.");

        Logger::Info(
            "Authentication Result : SUCCESS");
    }
    else
    {
        Logger::Info(
            "Challenge proof verification FAILED.");

        Logger::Info(
            "Authentication Result : FAILED");
    }

    return m_proofVerified;
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
	
	if (authenticated)
{
    result.sessionId = "TEMP";
    result.sessionKey = m_droneSessionKey;
    result.sessionLifetime = 600.0;
}

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

	result.authenticationTimeMs = authenticationTimeMs;

	result.reason =
    authenticated ?
    "RAP authentication successful." :
    "RAP authentication failed.";
		
	

return result;

}



    CryptoEngine* m_crypto = nullptr;

    KeyPair m_droneKeys;
    KeyPair m_vehicleKeys;

    SharedSecret m_droneSecret;
    SharedSecret m_vehicleSecret;

    SessionKey m_droneSessionKey;
    SessionKey m_vehicleSessionKey;

    bool m_vehicleAuthenticated = false;
    bool m_droneAuthenticated = false;
    bool m_proofVerified = false;
};


}

#endif
