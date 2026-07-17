#ifndef MESSAGES_H
#define MESSAGES_H

#include <vector>
#include <cstdint>
#include <cstring>
#include <string>
#include "blockchain.h"

// ---------------------------------------------------------------------------
// Simple binary wire format shared by all nodes. Each message starts with a
// 1-byte type tag followed by type-specific fields. Helper Put*/Get* methods
// implement a tiny cursor-based (de)serializer - no external dependency
// needed, keeps the packet payloads compact for realistic overhead numbers.
// ---------------------------------------------------------------------------
enum class MsgType : uint8_t
{
  AUTH_REQUEST     = 0x10,
  AUTH_ACCEPT      = 0x11,
  AUTH_REJECT      = 0x12,
  PBFT_PRE_PREPARE = 0x20,
  PBFT_PREPARE     = 0x21,
  PBFT_COMMIT      = 0x22,
  BLOCK_SYNC       = 0x23,
  DATA_MSG         = 0x30,
  ATTACK_DOS       = 0x41,
};

class ByteWriter
{
public:
  std::vector<uint8_t> buf;
  void U8 (uint8_t v) { buf.push_back (v); }
  void U32 (uint32_t v) { for (int i=0;i<4;++i) buf.push_back((v>>(8*i))&0xFF); }
  void U64 (uint64_t v) { for (int i=0;i<8;++i) buf.push_back((v>>(8*i))&0xFF); }
  void Dbl (double v) { uint8_t r[8]; std::memcpy(r,&v,8); buf.insert(buf.end(),r,r+8); }
  void Str (const std::string &s) { U32((uint32_t)s.size()); buf.insert(buf.end(), s.begin(), s.end()); }
  void Bytes (const std::vector<uint8_t> &b) { U32((uint32_t)b.size()); buf.insert(buf.end(), b.begin(), b.end()); }
};

class ByteReader
{
public:
  const std::vector<uint8_t> &buf;
  size_t pos = 0;
  explicit ByteReader (const std::vector<uint8_t> &b) : buf (b) {}
  uint8_t U8 () { return buf[pos++]; }
  uint32_t U32 () { uint32_t v=0; for(int i=0;i<4;++i) v |= (uint32_t)buf[pos++] << (8*i); return v; }
  uint64_t U64 () { uint64_t v=0; for(int i=0;i<8;++i) v |= (uint64_t)buf[pos++] << (8*i); return v; }
  double Dbl () { double v; std::memcpy(&v, &buf[pos], 8); pos += 8; return v; }
  std::string Str () { uint32_t n=U32(); std::string s(buf.begin()+pos, buf.begin()+pos+n); pos+=n; return s; }
  std::vector<uint8_t> Bytes () { uint32_t n=U32(); std::vector<uint8_t> b(buf.begin()+pos, buf.begin()+pos+n); pos+=n; return b; }
  bool Done () const { return pos >= buf.size (); }
};

// ---- AUTH_REQUEST: node -> RSU ------------------------------------------
struct AuthRequestMsg
{
  uint32_t nodeId; std::string nodeType; double timestamp; uint64_t nonce;
  std::vector<uint8_t> signature;   // node's sig over (nodeId,nodeType,timestamp,nonce)
  Certificate cert;                 // CA-issued certificate carried along

  std::vector<uint8_t> Serialize () const
  {
    ByteWriter w; w.U8((uint8_t)MsgType::AUTH_REQUEST);
    w.U32(nodeId); w.Str(nodeType); w.Dbl(timestamp); w.U64(nonce); w.Bytes(signature);
    w.U32(cert.nodeId); w.Str(cert.nodeType); w.Bytes(cert.publicKey);
    w.Dbl(cert.issuedAt); w.Dbl(cert.expiresAt); w.Bytes(cert.caSignature);
    return w.buf;
  }
  static AuthRequestMsg Parse (ByteReader &r)
  {
    AuthRequestMsg m;
    m.nodeId=r.U32(); m.nodeType=r.Str(); m.timestamp=r.Dbl(); m.nonce=r.U64(); m.signature=r.Bytes();
    m.cert.nodeId=r.U32(); m.cert.nodeType=r.Str(); m.cert.publicKey=r.Bytes();
    m.cert.issuedAt=r.Dbl(); m.cert.expiresAt=r.Dbl(); m.cert.caSignature=r.Bytes();
    return m;
  }
};

// ---- AUTH_ACCEPT: RSU -> node --------------------------------------------
struct AuthAcceptMsg
{
  uint32_t nodeId; uint64_t blockIndex; std::vector<uint8_t> blockHash; double authLatencySeed;

  std::vector<uint8_t> Serialize () const
  {
    ByteWriter w; w.U8((uint8_t)MsgType::AUTH_ACCEPT);
    w.U32(nodeId); w.U64(blockIndex); w.Bytes(blockHash); w.Dbl(authLatencySeed);
    return w.buf;
  }
  static AuthAcceptMsg Parse (ByteReader &r)
  {
    AuthAcceptMsg m; m.nodeId=r.U32(); m.blockIndex=r.U64(); m.blockHash=r.Bytes(); m.authLatencySeed=r.Dbl();
    return m;
  }
};

// ---- AUTH_REJECT: RSU -> node --------------------------------------------
struct AuthRejectMsg
{
  uint32_t nodeId; std::string reason;
  std::vector<uint8_t> Serialize () const
  { ByteWriter w; w.U8((uint8_t)MsgType::AUTH_REJECT); w.U32(nodeId); w.Str(reason); return w.buf; }
  static AuthRejectMsg Parse (ByteReader &r)
  { AuthRejectMsg m; m.nodeId=r.U32(); m.reason=r.Str(); return m; }
};

// ---- PBFT messages: RSU <-> RSU ------------------------------------------
struct PrePrepareMsg
{
  uint64_t view; uint64_t seq; Block block; uint32_t primaryId; std::vector<uint8_t> primarySig;

  std::vector<uint8_t> Serialize () const
  {
    ByteWriter w; w.U8((uint8_t)MsgType::PBFT_PRE_PREPARE);
    w.U64(view); w.U64(seq);
    w.U64(block.index); w.U64(block.viewNumber); w.U64(block.sequenceNumber);
    w.Bytes(block.prevHash); w.Dbl(block.timestamp); w.U32(block.proposerRsuId);
    w.U32((uint32_t)block.transactions.size());
    for (auto &tx: block.transactions)
      {
        w.U32(tx.nodeId); w.Str(tx.nodeType); w.Str(tx.txType);
        w.Dbl(tx.timestamp); w.U64(tx.nonce); w.Bytes(tx.signature);
      }
    w.U32(primaryId); w.Bytes(primarySig);
    return w.buf;
  }
  static PrePrepareMsg Parse (ByteReader &r)
  {
    PrePrepareMsg m; m.view=r.U64(); m.seq=r.U64();
    m.block.index=r.U64(); m.block.viewNumber=r.U64(); m.block.sequenceNumber=r.U64();
    m.block.prevHash=r.Bytes(); m.block.timestamp=r.Dbl(); m.block.proposerRsuId=r.U32();
    uint32_t n=r.U32();
    for (uint32_t i=0;i<n;++i)
      {
        Transaction tx; tx.nodeId=r.U32(); tx.nodeType=r.Str(); tx.txType=r.Str();
        tx.timestamp=r.Dbl(); tx.nonce=r.U64(); tx.signature=r.Bytes();
        m.block.transactions.push_back(tx);
      }
    m.primaryId=r.U32(); m.primarySig=r.Bytes();
    return m;
  }
};

struct VoteMsg // used for both PREPARE and COMMIT (same shape)
{
  MsgType type; uint64_t view; uint64_t seq; std::vector<uint8_t> blockHash;
  uint32_t rsuId; std::vector<uint8_t> signature;

  std::vector<uint8_t> Serialize () const
  {
    ByteWriter w; w.U8((uint8_t)type); w.U64(view); w.U64(seq); w.Bytes(blockHash);
    w.U32(rsuId); w.Bytes(signature);
    return w.buf;
  }
  static VoteMsg Parse (ByteReader &r, MsgType t)
  {
    VoteMsg m; m.type=t; m.view=r.U64(); m.seq=r.U64(); m.blockHash=r.Bytes();
    m.rsuId=r.U32(); m.signature=r.Bytes();
    return m;
  }
};

// ---- BLOCK_SYNC: RSU -> RSU, broadcast right after a block is committed --
// Lets any RSU whose own concurrently-proposed block at the same index
// *lost* the PBFT race (a real possibility in this multi-proposer lite
// model - see rsu-app.h) directly adopt the winning chain state instead of
// being permanently stuck waiting on a proposal that will never reach
// quorum. Simplification: the receiver trusts the sender and re-validates
// only chain linkage (prevHash/index), not the full commit-signature set;
// a hardened version would re-verify all quorum signatures before adopting.
struct BlockSyncMsg
{
  Block block;

  std::vector<uint8_t> Serialize () const
  {
    ByteWriter w; w.U8((uint8_t)MsgType::BLOCK_SYNC);
    w.U64(block.index); w.U64(block.viewNumber); w.U64(block.sequenceNumber);
    w.Bytes(block.prevHash); w.Dbl(block.timestamp); w.U32(block.proposerRsuId);
    w.U32((uint32_t)block.transactions.size());
    for (auto &tx: block.transactions)
      {
        w.U32(tx.nodeId); w.Str(tx.nodeType); w.Str(tx.txType);
        w.Dbl(tx.timestamp); w.U64(tx.nonce); w.Bytes(tx.signature);
      }
    return w.buf;
  }
  static BlockSyncMsg Parse (ByteReader &r)
  {
    BlockSyncMsg m;
    m.block.index=r.U64(); m.block.viewNumber=r.U64(); m.block.sequenceNumber=r.U64();
    m.block.prevHash=r.Bytes(); m.block.timestamp=r.Dbl(); m.block.proposerRsuId=r.U32();
    uint32_t n=r.U32();
    for (uint32_t i=0;i<n;++i)
      {
        Transaction tx; tx.nodeId=r.U32(); tx.nodeType=r.Str(); tx.txType=r.Str();
        tx.timestamp=r.Dbl(); tx.nonce=r.U64(); tx.signature=r.Bytes();
        m.block.transactions.push_back(tx);
      }
    return m;
  }
};

// ---- DATA_MSG: node -> RSU, HMAC-authenticated with session token -------
struct DataMsg
{
  uint32_t nodeId; double timestamp; std::string payload; std::vector<uint8_t> hmacTag;
  std::vector<uint8_t> Serialize () const
  {
    ByteWriter w; w.U8((uint8_t)MsgType::DATA_MSG);
    w.U32(nodeId); w.Dbl(timestamp); w.Str(payload); w.Bytes(hmacTag);
    return w.buf;
  }
  static DataMsg Parse (ByteReader &r)
  {
    DataMsg m; m.nodeId=r.U32(); m.timestamp=r.Dbl(); m.payload=r.Str(); m.hmacTag=r.Bytes();
    return m;
  }
};

#endif // MESSAGES_H
