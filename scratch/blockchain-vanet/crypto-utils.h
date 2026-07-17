#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <string>
#include <vector>
#include <cstdint>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

// ---------------------------------------------------------------------------
// CryptoUtils
//
// Thin wrapper around OpenSSL's EVP API providing the cryptographic
// primitives used throughout the simulation:
//   - ECDSA (secp256r1) key generation, signing, verification
//   - SHA-256 hashing (used for blockchain block hashes / tx digests)
//   - HMAC-SHA256 (used for session-token-authenticated data messages)
//
// All operations are *real* cryptographic operations (not mocked), so the
// simulation produces meaningful CPU-cost / latency numbers for the paper.
// ---------------------------------------------------------------------------
class CryptoUtils
{
public:
  struct KeyPair
  {
    EVP_PKEY *pkey = nullptr; // contains both private + public key material
  };

  // Generate a new ECDSA (prime256v1 / secp256r1) key pair.
  static KeyPair GenerateKeyPair ();

  // Free a key pair's underlying OpenSSL structures.
  static void FreeKeyPair (KeyPair &kp);

  // Serialize the public key to a raw uncompressed point (as bytes),
  // used as the node's "public identity" carried in certificates / packets.
  static std::vector<uint8_t> ExportPublicKey (const KeyPair &kp);

  // Reconstruct an EVP_PKEY (public-key only) from raw bytes exported above.
  static EVP_PKEY *ImportPublicKey (const std::vector<uint8_t> &raw);

  // SHA-256 hash of an arbitrary byte buffer, returned as 32 raw bytes.
  static std::vector<uint8_t> Sha256 (const std::vector<uint8_t> &data);
  static std::vector<uint8_t> Sha256 (const std::string &data);

  // Sign `data` with the private key in kp. Returns DER-encoded signature.
  static std::vector<uint8_t> Sign (const KeyPair &kp, const std::vector<uint8_t> &data);

  // Verify `signature` over `data` using the given public key.
  static bool Verify (EVP_PKEY *pubKey, const std::vector<uint8_t> &data,
                       const std::vector<uint8_t> &signature);

  // HMAC-SHA256(key, data) -> 32 raw bytes. Used for session-authenticated
  // data-plane messages once a vehicle/drone has an active session token.
  static std::vector<uint8_t> HmacSha256 (const std::vector<uint8_t> &key,
                                           const std::vector<uint8_t> &data);

  // Convenience: hex-encode a byte buffer for logging / packet framing.
  static std::string ToHex (const std::vector<uint8_t> &data);
  static std::vector<uint8_t> FromHex (const std::string &hex);
};

#endif // CRYPTO_UTILS_H
