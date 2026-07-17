#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include <string>
#include <vector>
#include <cstdint>
#include <map>
#include "crypto-utils.h"

// ---------------------------------------------------------------------------
// Certificate: a minimal X.509-like structure issued by a simulated
// Certificate Authority (CA) at network bootstrap. Each vehicle/drone gets
// one; RSUs verify it (CA signature) before accepting an auth request.
// ---------------------------------------------------------------------------
struct Certificate
{
  uint32_t nodeId;
  std::string nodeType;             // "vehicle" | "drone" | "rsu"
  std::vector<uint8_t> publicKey;   // node's ECDSA public key (raw point)
  double issuedAt;
  double expiresAt;
  std::vector<uint8_t> caSignature; // CA's signature over the above fields

  std::vector<uint8_t> SerializeForSigning () const;
};

// ---------------------------------------------------------------------------
// Transaction: one authentication (or data-attestation) event recorded on
// the ledger. Signed by the requesting node, later bundled into a Block by
// the PBFT primary RSU.
// ---------------------------------------------------------------------------
struct Transaction
{
  uint32_t nodeId;
  std::string nodeType;
  std::string txType;          // "AUTH" | "REVOKE"
  double timestamp;
  uint64_t nonce;               // anti-replay
  std::vector<uint8_t> signature; // node's signature over the fields above

  std::vector<uint8_t> SerializeForSigning () const;
  std::vector<uint8_t> Hash () const;
};

// ---------------------------------------------------------------------------
// Block: PBFT-committed batch of transactions.
// ---------------------------------------------------------------------------
struct Block
{
  uint64_t index = 0;
  uint64_t viewNumber = 0;
  uint64_t sequenceNumber = 0;
  std::vector<uint8_t> prevHash;
  std::vector<Transaction> transactions;
  double timestamp = 0.0;
  uint32_t proposerRsuId = 0;

  // Collected during PBFT COMMIT phase: rsuId -> commit-vote signature.
  std::map<uint32_t, std::vector<uint8_t>> commitSignatures;

  std::vector<uint8_t> ComputeHash () const;   // hash over header + tx digests
  std::vector<uint8_t> TransactionsDigest () const;
};

// Session token = HMAC(blockHash, nodeId). Free function so that vehicle/
// drone apps (which don't hold a full Blockchain instance) can derive the
// same token independently once they learn the committed block's hash from
// an AUTH_ACCEPT message.
std::vector<uint8_t> DeriveSessionTokenFromHash (uint32_t nodeId,
                                                  const std::vector<uint8_t> &blockHash);

// ---------------------------------------------------------------------------
// Blockchain: append-only ledger maintained identically by every RSU once
// PBFT commits a block. Provides validation + session-token derivation.
// ---------------------------------------------------------------------------
class Blockchain
{
public:
  Blockchain ();

  const Block &GetLatestBlock () const;
  uint64_t Height () const { return m_chain.size (); }

  // Validates linkage (prevHash) before appending; returns false on mismatch.
  bool AppendBlock (const Block &b);

  bool HasAuthenticated (uint32_t nodeId) const;
  uint64_t GetLastNonce (uint32_t nodeId) const;

  // Session token = HMAC(blockHash, nodeId) - both vehicle and RSU can derive
  // this independently once the AUTH block is committed, without another
  // round trip.
  std::vector<uint8_t> DeriveSessionToken (uint32_t nodeId, const Block &authBlock) const;

  const std::vector<Block> &Chain () const { return m_chain; }

private:
  std::vector<Block> m_chain;
  std::map<uint32_t, uint64_t> m_lastNonceByNode;
};

#endif // BLOCKCHAIN_H
