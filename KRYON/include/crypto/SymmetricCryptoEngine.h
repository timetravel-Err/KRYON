#ifndef KRYON_SYMMETRIC_CRYPTO_ENGINE_H
#define KRYON_SYMMETRIC_CRYPTO_ENGINE_H

#include "CryptoTypes.h"
#include "../core/ExperimentConfig.h"
#include "../simulation/SimulationContext.h"
#include "../core/Logger.h"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <stdexcept>
#include <string>
#include <vector>

namespace kryon
{

class SymmetricCryptoEngine
{
public:

    SymmetricCryptoEngine(
        const ExperimentConfig& config,
        SimulationContext& context)
        : m_config(config),
          m_context(context)
    {
    }

    void Initialize()
    {
        Logger::Info(
            "Symmetric Crypto Engine initialized.");
    }

    /*
     * ------------------------------------------------------
     * AES-256-GCM Encryption
     * ------------------------------------------------------
     */

    Ciphertext Encrypt(
        const ByteArray& plaintext,
        const SessionKey& key,
        const ByteArray& aad = ByteArray())
    {
        constexpr std::size_t KEY_SIZE = 32;
        constexpr std::size_t NONCE_SIZE = 12;
        constexpr std::size_t TAG_SIZE = 16;

        if (key.bytes.data.size() != KEY_SIZE)
        {
            throw std::runtime_error(
                "AES-256-GCM requires a 32-byte key.");
        }

        Ciphertext result;

        result.nonce.bytes.data.resize(
            NONCE_SIZE);

        result.tag.bytes.data.resize(
            TAG_SIZE);

        if (RAND_bytes(
                result.nonce.bytes.data.data(),
                NONCE_SIZE) != 1)
        {
            throw std::runtime_error(
                "Failed to generate AES-GCM nonce.");
        }

        EVP_CIPHER_CTX* ctx =
            EVP_CIPHER_CTX_new();

        if (ctx == nullptr)
        {
            throw std::runtime_error(
                "Failed to create EVP context.");
        }

        try
        {
            if (EVP_EncryptInit_ex(
                    ctx,
                    EVP_aes_256_gcm(),
                    nullptr,
                    nullptr,
                    nullptr) != 1)
            {
                throw std::runtime_error(
                    "AES-GCM initialization failed.");
            }

            if (EVP_CIPHER_CTX_ctrl(
                    ctx,
                    EVP_CTRL_GCM_SET_IVLEN,
                    NONCE_SIZE,
                    nullptr) != 1)
            {
                throw std::runtime_error(
                    "Failed to set GCM nonce length.");
            }

            if (EVP_EncryptInit_ex(
                    ctx,
                    nullptr,
                    nullptr,
                    key.bytes.data.data(),
                    result.nonce.bytes.data.data()) != 1)
            {
                throw std::runtime_error(
                    "Failed to initialize AES-GCM key.");
            }

            int outputLength = 0;

            /*
             * Authenticate additional data.
             */

            if (!aad.data.empty())
            {
                if (EVP_EncryptUpdate(
                        ctx,
                        nullptr,
                        &outputLength,
                        aad.data.data(),
                        aad.data.size()) != 1)
                {
                    throw std::runtime_error(
                        "AES-GCM AAD processing failed.");
                }
            }

            /*
             * Allocate ciphertext buffer.
             */

            result.bytes.data.resize(
                plaintext.data.size());

            int encryptedLength = 0;

            if (!plaintext.data.empty())
            {
                if (EVP_EncryptUpdate(
                        ctx,
                        result.bytes.data.data(),
                        &encryptedLength,
                        plaintext.data.data(),
                        plaintext.data.size()) != 1)
                {
                    throw std::runtime_error(
                        "AES-GCM encryption failed.");
                }
            }

            int finalLength = 0;

            if (EVP_EncryptFinal_ex(
                    ctx,
                    result.bytes.data.data() +
                        encryptedLength,
                    &finalLength) != 1)
            {
                throw std::runtime_error(
                    "AES-GCM finalization failed.");
            }

            result.bytes.data.resize(
                encryptedLength + finalLength);

            if (EVP_CIPHER_CTX_ctrl(
                    ctx,
                    EVP_CTRL_GCM_GET_TAG,
                    TAG_SIZE,
                    result.tag.bytes.data.data()) != 1)
            {
                throw std::runtime_error(
                    "Failed to obtain GCM authentication tag.");
            }

            EVP_CIPHER_CTX_free(ctx);

            return result;
        }
        catch (...)
        {
            EVP_CIPHER_CTX_free(ctx);
            throw;
        }
    }

    /*
     * ------------------------------------------------------
     * AES-256-GCM Decryption
     * ------------------------------------------------------
     */

    ByteArray Decrypt(
        const Ciphertext& ciphertext,
        const SessionKey& key,
        const ByteArray& aad = ByteArray())
    {
        constexpr std::size_t KEY_SIZE = 32;
        constexpr std::size_t NONCE_SIZE = 12;
        constexpr std::size_t TAG_SIZE = 16;

        if (key.bytes.data.size() != KEY_SIZE)
        {
            throw std::runtime_error(
                "AES-256-GCM requires a 32-byte key.");
        }

        if (ciphertext.nonce.bytes.data.size() !=
            NONCE_SIZE)
        {
            throw std::runtime_error(
                "Invalid AES-GCM nonce size.");
        }

        if (ciphertext.tag.bytes.data.size() !=
            TAG_SIZE)
        {
            throw std::runtime_error(
                "Invalid AES-GCM authentication tag size.");
        }

        EVP_CIPHER_CTX* ctx =
            EVP_CIPHER_CTX_new();

        if (ctx == nullptr)
        {
            throw std::runtime_error(
                "Failed to create EVP context.");
        }

        try
        {
            if (EVP_DecryptInit_ex(
                    ctx,
                    EVP_aes_256_gcm(),
                    nullptr,
                    nullptr,
                    nullptr) != 1)
            {
                throw std::runtime_error(
                    "AES-GCM initialization failed.");
            }

            if (EVP_CIPHER_CTX_ctrl(
                    ctx,
                    EVP_CTRL_GCM_SET_IVLEN,
                    NONCE_SIZE,
                    nullptr) != 1)
            {
                throw std::runtime_error(
                    "Failed to set GCM nonce length.");
            }

            if (EVP_DecryptInit_ex(
                    ctx,
                    nullptr,
                    nullptr,
                    key.bytes.data.data(),
                    ciphertext.nonce.bytes.data.data()) != 1)
            {
                throw std::runtime_error(
                    "Failed to initialize AES-GCM key.");
            }

            int outputLength = 0;

            /*
             * Authenticate additional data.
             */

            if (!aad.data.empty())
            {
                if (EVP_DecryptUpdate(
                        ctx,
                        nullptr,
                        &outputLength,
                        aad.data.data(),
                        aad.data.size()) != 1)
                {
                    throw std::runtime_error(
                        "AES-GCM AAD processing failed.");
                }
            }

            ByteArray plaintext;

            plaintext.data.resize(
                ciphertext.bytes.data.size());

            int decryptedLength = 0;

            if (!ciphertext.bytes.data.empty())
            {
                if (EVP_DecryptUpdate(
                        ctx,
                        plaintext.data.data(),
                        &decryptedLength,
                        ciphertext.bytes.data.data(),
                        ciphertext.bytes.data.size()) != 1)
                {
                    throw std::runtime_error(
                        "AES-GCM decryption failed.");
                }
            }

            /*
             * Supply authentication tag.
             */

            if (EVP_CIPHER_CTX_ctrl(
                    ctx,
                    EVP_CTRL_GCM_SET_TAG,
                    TAG_SIZE,
                    const_cast<unsigned char*>(
                        ciphertext.tag.bytes.data.data())) != 1)
            {
                throw std::runtime_error(
                    "Failed to set GCM authentication tag.");
            }

            int finalLength = 0;

            /*
             * EVP_DecryptFinal_ex returns 0 if the
             * authentication tag is invalid.
             */

            if (EVP_DecryptFinal_ex(
                    ctx,
                    plaintext.data.data() +
                        decryptedLength,
                    &finalLength) != 1)
            {
                //EVP_CIPHER_CTX_free(ctx);

                throw std::runtime_error(
                    "AES-GCM authentication failed.");
            }

            plaintext.data.resize(
                decryptedLength + finalLength);

            EVP_CIPHER_CTX_free(ctx);

            return plaintext;
        }
        catch (...)
        {
            EVP_CIPHER_CTX_free(ctx);
            throw;
        }
    }

    void Finalize()
    {
        Logger::Info(
            "Symmetric Crypto Engine finalized.");
    }

private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;
};

}

#endif
