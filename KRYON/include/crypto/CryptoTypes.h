#ifndef KRYON_CRYPTO_TYPES_H
#define KRYON_CRYPTO_TYPES_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : CryptoTypes.h
 *
 * Common cryptographic data structures shared across
 * the framework.
 *
 * This file contains data types only.
 * Cryptographic implementations remain inside crypto engines.
 * ----------------------------------------------------------
 */

#include <vector>
#include <cstdint>

namespace kryon
{

/* ----------------------------------------------------------
 * Generic Byte Container
 * ---------------------------------------------------------- */

struct ByteArray
{
    std::vector<uint8_t> data;
};

/* ----------------------------------------------------------
 * Hash Value
 * ---------------------------------------------------------- */

struct HashValue
{
    ByteArray bytes;
};

/* ----------------------------------------------------------
 * Public Key
 * ---------------------------------------------------------- */

struct PublicKey
{
    ByteArray bytes;
};

/* ----------------------------------------------------------
 * Private Key
 * ---------------------------------------------------------- */

struct PrivateKey
{
    ByteArray bytes;
};

/* ----------------------------------------------------------
 * Key Pair
 * ---------------------------------------------------------- */

struct KeyPair
{
    PublicKey publicKey;
    PrivateKey privateKey;
};

/* ----------------------------------------------------------
 * Shared Secret
 * ---------------------------------------------------------- */

struct SharedSecret
{
    ByteArray bytes;
};

/* ----------------------------------------------------------
 * Session Key
 * ---------------------------------------------------------- */

struct SessionKey
{
    ByteArray bytes;
};

/* ----------------------------------------------------------
 * AES-GCM Nonce
 * ---------------------------------------------------------- */

struct GCMNonce
{
    ByteArray bytes;
};

/* ----------------------------------------------------------
 * Nonce
 *
 * Used by the generic Ciphertext structure.
 * ---------------------------------------------------------- */

struct Nonce
{
    ByteArray bytes;
};

/* ----------------------------------------------------------
 * AES-GCM Authentication Tag
 * ---------------------------------------------------------- */

struct AuthenticationTag
{
    ByteArray bytes;
};

/* ----------------------------------------------------------
 * Generic Ciphertext
 *
 * Contains:
 *
 *   bytes -> encrypted payload
 *   nonce -> AES-GCM nonce / IV
 *   tag   -> AES-GCM authentication tag
 *
 * This structure is used directly by
 * SymmetricCryptoEngine.
 * ---------------------------------------------------------- */

struct Ciphertext
{
    ByteArray bytes;

    Nonce nonce;

    AuthenticationTag tag;
};

/* ----------------------------------------------------------
 * AES-GCM Encryption Result
 *
 * Kept as a higher-level wrapper for future use.
 * ---------------------------------------------------------- */

struct AEADCiphertext
{
    Ciphertext ciphertext;

    GCMNonce nonce;

    AuthenticationTag tag;
};

/* ----------------------------------------------------------
 * Digital Signature
 * ---------------------------------------------------------- */

struct Signature
{
    ByteArray bytes;
};

} // namespace kryon

#endif // KRYON_CRYPTO_TYPES_H