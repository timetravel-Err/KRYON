#include "../../KRYON/include/authentication/network/AuthenticationMessageCodec.h"
#include "../../KRYON/include/core/Logger.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"

#include <iostream>

using namespace ns3;

int main()
{
    kryon::Logger::Info(
        "==========================================");

    kryon::Logger::Info(
        "KRYON Authentication Codec Test");

    kryon::Logger::Info(
        "==========================================");


    /* ======================================================
     * AUTH_REQUEST TEST
     * ====================================================== */

    kryon::AuthRequestPacket request;

    request.requestId = "TEST-REQ-1";
    request.sourceNode = 2;
    request.destinationNode = 10;
    request.timestamp = 1.25;

    request.nonce =
        std::vector<uint8_t>(
            32,
            0x11);

    request.authenticationData =
        std::vector<uint8_t>(
            100,
            0x22);

    request.integrityHash =
        std::vector<uint8_t>(
            32,
            0x33);


    Ptr<Packet> requestPacket =
        kryon::AuthenticationMessageCodec::Encode(
            request);

    kryon::AuthRequestPacket decodedRequest =
        kryon::AuthenticationMessageCodec::DecodeRequest(
            requestPacket);


    std::cout
        << "AUTH_REQUEST encoded size = "
        << requestPacket->GetSize()
        << " bytes"
        << std::endl;

    std::cout
        << "AUTH_REQUEST decoded ID = "
        << decodedRequest.requestId
        << std::endl;


    /* ======================================================
     * AUTH_CHALLENGE TEST
     * ====================================================== */

    kryon::AuthChallengePacket challenge;

    challenge.requestId = "TEST-REQ-1";
    challenge.sourceNode = 10;
    challenge.destinationNode = 2;
    challenge.timestamp = 1.30;

    challenge.challenge =
        std::vector<uint8_t>(
            32,
            0x44);

    challenge.senderPublicKey =
        std::vector<uint8_t>(
            91,
            0x55);

    challenge.signature =
        std::vector<uint8_t>(
            72,
            0x66);

    challenge.integrityHash =
        std::vector<uint8_t>(
            32,
            0x77);


    Ptr<Packet> challengePacket =
        kryon::AuthenticationMessageCodec::Encode(
            challenge);

    kryon::AuthChallengePacket decodedChallenge =
        kryon::AuthenticationMessageCodec::DecodeChallenge(
            challengePacket);


    std::cout
        << "AUTH_CHALLENGE encoded size = "
        << challengePacket->GetSize()
        << " bytes"
        << std::endl;

    std::cout
        << "AUTH_CHALLENGE decoded ID = "
        << decodedChallenge.requestId
        << std::endl;


    /* ======================================================
     * AUTH_RESPONSE TEST
     * ====================================================== */

    kryon::AuthResponsePacket response;

    response.requestId = "TEST-REQ-1";
    response.sourceNode = 2;
    response.destinationNode = 10;
    response.timestamp = 1.40;

    response.challenge =
        std::vector<uint8_t>(
            32,
            0x44);

    response.proof =
        std::vector<uint8_t>(
            32,
            0x88);

    response.senderPublicKey =
        std::vector<uint8_t>(
            91,
            0x99);

    response.signature =
        std::vector<uint8_t>(
            72,
            0xaa);

    response.integrityHash =
        std::vector<uint8_t>(
            32,
            0xbb);


    Ptr<Packet> responsePacket =
        kryon::AuthenticationMessageCodec::Encode(
            response);

    kryon::AuthResponsePacket decodedResponse =
        kryon::AuthenticationMessageCodec::DecodeResponse(
            responsePacket);


    std::cout
        << "AUTH_RESPONSE encoded size = "
        << responsePacket->GetSize()
        << " bytes"
        << std::endl;

    std::cout
        << "AUTH_RESPONSE decoded ID = "
        << decodedResponse.requestId
        << std::endl;


    /* ======================================================
     * AUTH_CONFIRM TEST
     * ====================================================== */

    kryon::AuthConfirmPacket confirm;

    confirm.requestId = "TEST-REQ-1";
    confirm.sourceNode = 10;
    confirm.destinationNode = 2;
    confirm.timestamp = 1.50;

    confirm.authenticationSuccessful = true;

    confirm.message =
        "RAP authentication successful.";

    confirm.integrityHash =
        std::vector<uint8_t>(
            32,
            0xcc);


    Ptr<Packet> confirmPacket =
        kryon::AuthenticationMessageCodec::Encode(
            confirm);

    kryon::AuthConfirmPacket decodedConfirm =
        kryon::AuthenticationMessageCodec::DecodeConfirm(
            confirmPacket);


    std::cout
        << "AUTH_CONFIRM encoded size = "
        << confirmPacket->GetSize()
        << " bytes"
        << std::endl;

    std::cout
        << "AUTH_CONFIRM decoded ID = "
        << decodedConfirm.requestId
        << std::endl;

    std::cout
        << "AUTH_CONFIRM success = "
        << decodedConfirm.authenticationSuccessful
        << std::endl;


    std::cout
        << "=========================================="
        << std::endl;

    std::cout
        << "Authentication codec test completed."
        << std::endl;

    std::cout
        << "=========================================="
        << std::endl;


    return 0;
}
