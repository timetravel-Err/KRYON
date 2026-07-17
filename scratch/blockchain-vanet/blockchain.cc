#include "blockchain.h"
#include <cstring>

static void
AppendU32 (std::vector<uint8_t> &buf, uint32_t v)
{
  for (int i = 0; i < 4; ++i) buf.push_back (static_cast<uint8_t> ((v >> (8 * i)) & 0xFF));
}
static void
AppendU64 (std::vector<uint8_t> &buf, uint64_t v)
{
  for (int i = 0; i < 8; ++i) buf.push_back (static_cast<uint8_t> ((v >> (8 * i)) & 0xFF));
}
static void
AppendDouble (std::vector<uint8_t> &buf, double v)
{
  uint8_t raw[sizeof (double)];
  std::memcpy (raw, &v, sizeof (double));
  buf.insert (buf.end (), raw, raw + sizeof (double));
}
static void
AppendStr (std::vector<uint8_t> &buf, const std::string &s)
{
  AppendU32 (buf, static_cast<uint32_t> (s.size ()));
  buf.insert (buf.end (), s.begin (), s.end ());
}
static void
AppendBytes (std::vector<uint8_t> &buf, const std::vector<uint8_t> &b)
{
  AppendU32 (buf, static_cast<uint32_t> (b.size ()));
  buf.insert (buf.end (), b.begin (), b.end ());
}

std::vector<uint8_t>
Certificate::SerializeForSigning () const
{
  std::vector<uint8_t> buf;
  AppendU32 (buf, nodeId);
  AppendStr (buf, nodeType);
  AppendBytes (buf, publicKey);
  AppendDouble (buf, issuedAt);
  AppendDouble (buf, expiresAt);
  return buf;
}

std::vector<uint8_t>
Transaction::SerializeForSigning () const
{
  std::vector<uint8_t> buf;
  AppendU32 (buf, nodeId);
  AppendStr (buf, nodeType);
  AppendStr (buf, txType);
  AppendDouble (buf, timestamp);
  AppendU64 (buf, nonce);
  return buf;
}

std::vector<uint8_t>
Transaction::Hash () const
{
  std::vector<uint8_t> buf = SerializeForSigning ();
  AppendBytes (buf, signature);
  return CryptoUtils::Sha256 (buf);
}

std::vector<uint8_t>
Block::TransactionsDigest () const
{
  std::vector<uint8_t> buf;
  for (const auto &tx : transactions)
    {
      auto h = tx.Hash ();
      buf.insert (buf.end (), h.begin (), h.end ());
    }
  return CryptoUtils::Sha256 (buf);
}

std::vector<uint8_t>
Block::ComputeHash () const
{
  std::vector<uint8_t> buf;
  AppendU64 (buf, index);
  AppendU64 (buf, viewNumber);
  AppendU64 (buf, sequenceNumber);
  AppendBytes (buf, prevHash);
  auto txDigest = TransactionsDigest ();
  AppendBytes (buf, txDigest);
  AppendDouble (buf, timestamp);
  AppendU32 (buf, proposerRsuId);
  return CryptoUtils::Sha256 (buf);
}

Blockchain::Blockchain ()
{
  // Genesis block: no transactions, prevHash = all zero.
  Block genesis;
  genesis.index = 0;
  genesis.viewNumber = 0;
  genesis.sequenceNumber = 0;
  genesis.prevHash = std::vector<uint8_t> (32, 0);
  genesis.timestamp = 0.0;
  genesis.proposerRsuId = 0;
  m_chain.push_back (genesis);
}

const Block &
Blockchain::GetLatestBlock () const
{
  return m_chain.back ();
}

bool
Blockchain::AppendBlock (const Block &b)
{
  const Block &tip = GetLatestBlock ();
  auto tipHash = tip.ComputeHash ();
  if (b.prevHash != tipHash || b.index != tip.index + 1)
    {
      return false; // fork / stale proposal - reject
    }
  m_chain.push_back (b);
  for (const auto &tx : b.transactions)
    {
      if (tx.txType == "AUTH")
        {
          m_lastNonceByNode[tx.nodeId] = tx.nonce;
        }
    }
  return true;
}

bool
Blockchain::HasAuthenticated (uint32_t nodeId) const
{
  return m_lastNonceByNode.find (nodeId) != m_lastNonceByNode.end ();
}

uint64_t
Blockchain::GetLastNonce (uint32_t nodeId) const
{
  auto it = m_lastNonceByNode.find (nodeId);
  return (it == m_lastNonceByNode.end ()) ? 0 : it->second;
}

std::vector<uint8_t>
Blockchain::DeriveSessionToken (uint32_t nodeId, const Block &authBlock) const
{
  return DeriveSessionTokenFromHash (nodeId, authBlock.ComputeHash ());
}

std::vector<uint8_t>
DeriveSessionTokenFromHash (uint32_t nodeId, const std::vector<uint8_t> &blockHash)
{
  std::vector<uint8_t> nodeIdBytes;
  AppendU32 (nodeIdBytes, nodeId);
  return CryptoUtils::HmacSha256 (blockHash, nodeIdBytes);
}
