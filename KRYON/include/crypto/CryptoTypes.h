#ifndef KRYON_CRYPTO_TYPES_H
#define KRYON_CRYPTO_TYPES_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : CryptoTypes.h
 *
 * Description
 * -----------
 * Common cryptographic data structures shared across the
 * framework.
 *
 * This file intentionally contains only data types and no
 * cryptographic algorithms.
 *
 * Future engines (Hash, ECC, Random, PQC, etc.) operate on
 * these structures.
 *
 * Design Philosophy
 * -----------------
 * - No cryptographic implementation
 * - No OpenSSL/Crypto++ dependencies
 * - Strongly typed cryptographic objects
 * - Easy replacement of algorithms
 * ----------------------------------------------------------
 */

#include <vector>
#include <cstdint>

namespace kryon
{

/* ----------------------------------------------------------
 * Generic Byte Container
 * ----------------------------------------------------------*/

struct ByteArray
{
    std::vector<uint8_t> data;
};

/* ----------------------------------------------------------
 * Hash Value
 * ----------------------------------------------------------*/

struct HashValue
{
    ByteArray bytes;
};

/* ----------------------------------------------------------
 * Public Key
 * ----------------------------------------------------------*/

struct PublicKey
{
    ByteArray bytes;
};

/* ----------------------------------------------------------
 * Private Key
 * ----------------------------------------------------------*/

struct PrivateKey
{
    ByteArray bytes;
};

/* ----------------------------------------------------------
 * Key Pair
 * ----------------------------------------------------------*/

struct KeyPair
{
    PublicKey publicKey;
    PrivateKey privateKey;
};

/* ----------------------------------------------------------
 * Shared Secret
 * ----------------------------------------------------------*/

struct SharedSecret
{
    ByteArray bytes;
};

/* ----------------------------------------------------------
 * Digital Signature
 * ----------------------------------------------------------*/

struct Signature
{
    ByteArray bytes;
};

/* ----------------------------------------------------------
 * Nonce
 * ----------------------------------------------------------*/

struct Nonce
{
    ByteArray bytes;
};

} // namespace kryon

#endif
