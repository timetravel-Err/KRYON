#include "../../KRYON/include/crypto/CryptoTypes.h"
#include "../../KRYON/include/crypto/KeyGenerator.h"
#include "../../KRYON/include/crypto/ECCEngine.h"
#include "../../KRYON/include/core/ExperimentConfig.h"
#include "../../KRYON/include/simulation/SimulationContext.h"
#include "../../KRYON/include/core/Logger.h"

#include <iostream>

int main()
{
    kryon::Logger::Info(
        "==========================================");

    kryon::Logger::Info(
        "KRYON ECC TEST");

    kryon::Logger::Info(
        "==========================================");

    /*
     * ------------------------------------------------------
     * Create configuration and simulation context
     * ------------------------------------------------------
     */

    kryon::ExperimentConfig config;

    kryon::SimulationContext context;

    /*
     * ------------------------------------------------------
     * Initialize Key Generator
     * ------------------------------------------------------
     */

    kryon::KeyGenerator keyGenerator(
        config,
        context);

    keyGenerator.Initialize();

    /*
     * ------------------------------------------------------
     * Initialize ECC Engine
     * ------------------------------------------------------
     */

    kryon::ECCEngine ecc(
        config,
        context);

    ecc.Initialize();

    /*
     * ======================================================
     * TEST 1: ECC KEY GENERATION
     * ======================================================
     */

    std::cout
        << std::endl
        << "------------------------------------------"
        << std::endl;

    std::cout
        << "TEST 1: ECC KEY GENERATION"
        << std::endl;

    std::cout
        << "------------------------------------------"
        << std::endl;

    kryon::KeyPair keys =
        keyGenerator.GenerateKeyPair();

    std::cout
        << "Public key size  = "
        << keys.publicKey.bytes.data.size()
        << " bytes"
        << std::endl;

    std::cout
        << "Private key size = "
        << keys.privateKey.bytes.data.size()
        << " bytes"
        << std::endl;

    if (keys.publicKey.bytes.data.empty() ||
        keys.privateKey.bytes.data.empty())
    {
        std::cout
            << "KEY GENERATION: FAILED"
            << std::endl;

        return 1;
    }

    std::cout
        << "KEY GENERATION: SUCCESS"
        << std::endl;

    /*
     * ======================================================
     * TEST 2: ECDSA SIGNATURE
     * ======================================================
     */

    std::cout
        << std::endl
        << "------------------------------------------"
        << std::endl;

    std::cout
        << "TEST 2: ECDSA SIGNATURE"
        << std::endl;

    std::cout
        << "------------------------------------------"
        << std::endl;

    kryon::ByteArray message;

    std::string text =
        "KRYON ECC authentication test";

    message.data.assign(
        text.begin(),
        text.end());

    /*
     * ------------------------------------------------------
     * Sign message
     * ------------------------------------------------------
     */

    kryon::Signature signature =
        ecc.Sign(
            message,
            keys.privateKey);

    std::cout
        << "Signature size = "
        << signature.bytes.data.size()
        << " bytes"
        << std::endl;

    if (signature.bytes.data.empty())
    {
        std::cout
            << "ECDSA SIGN: FAILED"
            << std::endl;

        return 1;
    }

    std::cout
        << "ECDSA SIGN: SUCCESS"
        << std::endl;

    /*
     * ------------------------------------------------------
     * Verify original message
     * ------------------------------------------------------
     */

    bool verified =
        ecc.Verify(
            message,
            signature,
            keys.publicKey);

    std::cout
        << "ECDSA VERIFY original message: "
        << (verified ? "SUCCESS" : "FAILED")
        << std::endl;

    if (!verified)
    {
        return 1;
    }

    /*
     * ------------------------------------------------------
     * Modify message
     * ------------------------------------------------------
     */

    kryon::ByteArray modifiedMessage =
        message;

    modifiedMessage.data.push_back(
        0x01);

    /*
     * ------------------------------------------------------
     * Verify modified message
     * ------------------------------------------------------
     */

    bool modifiedVerified =
        ecc.Verify(
            modifiedMessage,
            signature,
            keys.publicKey);

    std::cout
        << "ECDSA VERIFY modified message: "
        << (modifiedVerified ? "FAILED" : "SUCCESS")
        << std::endl;

    if (modifiedVerified)
    {
        std::cout
            << "ERROR: modified message was accepted."
            << std::endl;

        return 1;
    }

    /*
     * ======================================================
     * TEST 3: ECDH SHARED SECRET
     * ======================================================
     */

    std::cout
        << std::endl
        << "------------------------------------------"
        << std::endl;

    std::cout
        << "TEST 3: ECDH SHARED SECRET"
        << std::endl;

    std::cout
        << "------------------------------------------"
        << std::endl;

    /*
     * ------------------------------------------------------
     * Generate Drone key pair
     * ------------------------------------------------------
     */

    std::cout
        << "Generating Drone key pair..."
        << std::endl;

    kryon::KeyPair droneKeys =
        keyGenerator.GenerateKeyPair();

    /*
     * ------------------------------------------------------
     * Generate Vehicle key pair
     * ------------------------------------------------------
     */

    std::cout
        << "Generating Vehicle key pair..."
        << std::endl;

    kryon::KeyPair vehicleKeys =
        keyGenerator.GenerateKeyPair();

    if (droneKeys.publicKey.bytes.data.empty() ||
        droneKeys.privateKey.bytes.data.empty() ||
        vehicleKeys.publicKey.bytes.data.empty() ||
        vehicleKeys.privateKey.bytes.data.empty())
    {
        std::cout
            << "ECDH KEY GENERATION: FAILED"
            << std::endl;

        return 1;
    }

    std::cout
        << "Drone public key size    = "
        << droneKeys.publicKey.bytes.data.size()
        << " bytes"
        << std::endl;

    std::cout
        << "Vehicle public key size  = "
        << vehicleKeys.publicKey.bytes.data.size()
        << " bytes"
        << std::endl;

    /*
     * ------------------------------------------------------
     * Drone derives shared secret using:
     *
     * Drone private key
     * Vehicle public key
     * ------------------------------------------------------
     */

    kryon::SharedSecret droneSecret =
        ecc.DeriveSharedSecret(
            droneKeys.privateKey,
            vehicleKeys.publicKey);

    /*
     * ------------------------------------------------------
     * Vehicle derives shared secret using:
     *
     * Vehicle private key
     * Drone public key
     * ------------------------------------------------------
     */

    kryon::SharedSecret vehicleSecret =
        ecc.DeriveSharedSecret(
            vehicleKeys.privateKey,
            droneKeys.publicKey);

    std::cout
        << "Drone shared secret size   = "
        << droneSecret.bytes.data.size()
        << " bytes"
        << std::endl;

    std::cout
        << "Vehicle shared secret size = "
        << vehicleSecret.bytes.data.size()
        << " bytes"
        << std::endl;

    /*
     * ------------------------------------------------------
     * Verify both secrets
     * ------------------------------------------------------
     */

    bool sharedSecretMatch =
        (droneSecret.bytes.data ==
         vehicleSecret.bytes.data);

    if (sharedSecretMatch)
    {
        std::cout
            << "ECDH SHARED SECRET: SUCCESS"
            << std::endl;
    }
    else
    {
        std::cout
            << "ECDH SHARED SECRET: FAILED"
            << std::endl;

        return 1;
    }

    /*
     * ======================================================
     * TEST 4: SESSION KEY DERIVATION
     * ======================================================
     */

    std::cout
        << std::endl
        << "------------------------------------------"
        << std::endl;

    std::cout
        << "TEST 4: SESSION KEY DERIVATION"
        << std::endl;

    std::cout
        << "------------------------------------------"
        << std::endl;

    /*
     * ------------------------------------------------------
     * Drone derives session key
     * ------------------------------------------------------
     */

    kryon::SessionKey droneSessionKey =
        ecc.DeriveSessionKey(
            droneSecret);

    /*
     * ------------------------------------------------------
     * Vehicle derives session key
     * ------------------------------------------------------
     */

    kryon::SessionKey vehicleSessionKey =
        ecc.DeriveSessionKey(
            vehicleSecret);

    std::cout
        << "Drone session key size   = "
        << droneSessionKey.bytes.data.size()
        << " bytes"
        << std::endl;

    std::cout
        << "Vehicle session key size = "
        << vehicleSessionKey.bytes.data.size()
        << " bytes"
        << std::endl;

    /*
     * ------------------------------------------------------
     * Compare session keys
     * ------------------------------------------------------
     */

    bool sessionKeyMatch =
        (droneSessionKey.bytes.data ==
         vehicleSessionKey.bytes.data);

    if (sessionKeyMatch)
    {
        std::cout
            << "SESSION KEY DERIVATION: SUCCESS"
            << std::endl;
    }
    else
    {
        std::cout
            << "SESSION KEY DERIVATION: FAILED"
            << std::endl;

        return 1;
    }

    /*
     * ======================================================
     * FINAL RESULT
     * ======================================================
     */

    ecc.Finalize();

    keyGenerator.Finalize();

    std::cout
        << std::endl
        << "=========================================="
        << std::endl;

    std::cout
        << "ALL ECC TESTS PASSED"
        << std::endl;

    std::cout
        << "=========================================="
        << std::endl;

    return 0;
}