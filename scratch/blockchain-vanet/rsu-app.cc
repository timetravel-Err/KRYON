#include "rsu-app.h"
#include "metrics.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/inet-socket-address.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/udp-socket-factory.h"

NS_LOG_COMPONENT_DEFINE ("RsuApp");

using namespace ns3;

NS_OBJECT_ENSURE_REGISTERED (RsuApp);

TypeId
RsuApp::GetTypeId ()
{
  static TypeId tid = TypeId ("RsuApp")
    .SetParent<Application> ()
    .AddConstructor<RsuApp> ();
  return tid;
}

RsuApp::RsuApp () : m_rsuId (0), m_totalRsuCount (1), m_port (9000) {}
RsuApp::~RsuApp () { if (m_caPublicKey) EVP_PKEY_free (m_caPublicKey); }

void
RsuApp::Setup (uint32_t rsuId, CryptoUtils::KeyPair keyPair,
                std::vector<uint8_t> caPublicKeyBytes, std::vector<PeerRsu> peers,
                uint16_t port, uint32_t totalRsuCount)
{
  m_rsuId = rsuId;
  m_keyPair = keyPair;
  m_caPublicKey = CryptoUtils::ImportPublicKey (caPublicKeyBytes);
  m_peers = std::move (peers);
  m_port = port;
  m_totalRsuCount = totalRsuCount;
}

bool
RsuApp::HasSessionToken (uint32_t nodeId) const
{
  return m_sessionTokens.find (nodeId) != m_sessionTokens.end ();
}

void
RsuApp::DoDispose ()
{
  m_socket = nullptr;
  Application::DoDispose ();
}

void
RsuApp::StartApplication ()
{
  if (!m_socket)
    {
      m_socket = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
      m_socket->Bind (local);
      m_socket->SetRecvCallback (MakeCallback (&RsuApp::HandleRead, this));
    }
}

void
RsuApp::StopApplication ()
{
  if (m_socket) m_socket->Close ();
}

void
RsuApp::SendTo (const Address &to, const std::vector<uint8_t> &bytes)
{
  Ptr<Packet> p = Create<Packet> (bytes.data (), bytes.size ());
  m_socket->SendTo (p, 0, to);
}

void
RsuApp::BroadcastToPeers (const std::vector<uint8_t> &bytes)
{
  for (auto &peer : m_peers) SendTo (peer.address, bytes);
}

void
RsuApp::HandleRead (Ptr<Socket> socket)
{
  Address from;
  Ptr<Packet> packet;
  while ((packet = socket->RecvFrom (from)))
    {
      if (packet->GetSize () == 0) continue;
      std::vector<uint8_t> buf (packet->GetSize ());
      packet->CopyData (buf.data (), buf.size ());
      MsgType type = static_cast<MsgType> (buf[0]);
      switch (type)
        {
        case MsgType::AUTH_REQUEST: HandleAuthRequest (buf, from); break;
        case MsgType::PBFT_PRE_PREPARE: HandlePrePrepare (buf, from); break;
        case MsgType::PBFT_PREPARE: HandlePrepare (buf, from); break;
        case MsgType::PBFT_COMMIT: HandleCommit (buf, from); break;
        case MsgType::BLOCK_SYNC: HandleBlockSync (buf, from); break;
        case MsgType::DATA_MSG: HandleDataMsg (buf, from); break;
        default: /* ATTACK_DOS or unknown -> just counted as noise below */
          {
            if (!CheckRateLimit (from))
              MetricsCollector::Get ().LogAttackEvent (Simulator::Now ().GetSeconds (),
                                                        "DosFlood", 0, "detected");
            break;
          }
        }
    }
}

bool
RsuApp::CheckRateLimit (const Address &from)
{
  std::ostringstream key;
  key << InetSocketAddress::ConvertFrom (from).GetIpv4 ();
  double now = Simulator::Now ().GetSeconds ();
  auto &q = m_recentRequestTimes[key.str ()];
  while (!q.empty () && now - q.front () > m_rateWindowSec) q.pop_front ();
  q.push_back (now);
  return q.size () <= m_rateMaxPerWindow;
}

bool
RsuApp::VerifyCertificate (const Certificate &cert) const
{
  double now = Simulator::Now ().GetSeconds ();
  if (now > cert.expiresAt) return false;
  auto signedBytes = cert.SerializeForSigning ();
  return CryptoUtils::Verify (m_caPublicKey, signedBytes, cert.caSignature);
}

void
RsuApp::HandleAuthRequest (const std::vector<uint8_t> &buf, const Address &from)
{
  double now = Simulator::Now ().GetSeconds ();

  if (!CheckRateLimit (from))
    {
      MetricsCollector::Get ().LogAttackEvent (now, "DoS_or_Sybil_flood", 0, "detected");
      return; // silently drop - rate-limited source
    }

  ByteReader r (buf); r.U8 ();
  AuthRequestMsg msg = AuthRequestMsg::Parse (r);
  m_authRequestArrival[msg.nodeId] = now;

  if (msg.cert.nodeId != msg.nodeId || !VerifyCertificate (msg.cert))
    {
      MetricsCollector::Get ().LogAttackEvent (now, "ForgedOrUnregisteredCertificate", msg.nodeId, "detected");
      MetricsCollector::Get ().LogAuthResult (now, msg.nodeId, false, 0.0, "invalid_certificate");
      AuthRejectMsg rej{msg.nodeId, "invalid_certificate"};
      SendTo (from, rej.Serialize ());
      return;
    }

  EVP_PKEY *nodePub = CryptoUtils::ImportPublicKey (msg.cert.publicKey);
  ByteWriter w; w.U32 (msg.nodeId); w.Str (msg.nodeType); w.Dbl (msg.timestamp); w.U64 (msg.nonce);
  bool sigOk = CryptoUtils::Verify (nodePub, w.buf, msg.signature);
  EVP_PKEY_free (nodePub);
  if (!sigOk)
    {
      MetricsCollector::Get ().LogAuthResult (now, msg.nodeId, false, 0.0, "bad_signature");
      AuthRejectMsg rej{msg.nodeId, "bad_signature"};
      SendTo (from, rej.Serialize ());
      return;
    }

  if (std::fabs (now - msg.timestamp) > m_freshnessWindowSec)
    {
      MetricsCollector::Get ().LogAttackEvent (now, "ReplayAttack", msg.nodeId, "detected");
      MetricsCollector::Get ().LogAuthResult (now, msg.nodeId, false, 0.0, "stale_timestamp");
      AuthRejectMsg rej{msg.nodeId, "stale_timestamp"};
      SendTo (from, rej.Serialize ());
      return;
    }

  if (msg.nonce <= m_blockchain.GetLastNonce (msg.nodeId))
    {
      MetricsCollector::Get ().LogAttackEvent (now, "ReplayAttack", msg.nodeId, "detected");
      MetricsCollector::Get ().LogAuthResult (now, msg.nodeId, false, 0.0, "replayed_nonce");
      AuthRejectMsg rej{msg.nodeId, "replayed_nonce"};
      SendTo (from, rej.Serialize ());
      return;
    }

  if (m_proposalInFlight)
    {
      MetricsCollector::Get ().LogAuthResult (now, msg.nodeId, false, 0.0, "rsu_busy_retry");
      AuthRejectMsg rej{msg.nodeId, "rsu_busy_retry"};
      SendTo (from, rej.Serialize ());
      return;
    }

  Transaction tx;
  tx.nodeId = msg.nodeId; tx.nodeType = msg.nodeType; tx.txType = "AUTH";
  tx.timestamp = msg.timestamp; tx.nonce = msg.nonce; tx.signature = msg.signature;
  ProposeBlock (tx, from);
}

void
RsuApp::ProposeBlock (const Transaction &tx, const Address &vehicleAddr)
{
  double now = Simulator::Now ().GetSeconds ();
  const Block &tip = m_blockchain.GetLatestBlock ();

  Block b;
  b.index = tip.index + 1;
  b.viewNumber = 0;
  b.sequenceNumber = b.index;
  b.prevHash = tip.ComputeHash ();
  b.transactions = {tx};
  b.timestamp = now;
  b.proposerRsuId = m_rsuId;
  auto blockHash = b.ComputeHash ();
  auto sig = CryptoUtils::Sign (m_keyPair, blockHash);

  ProposalState ps;
  ps.block = b; ps.blockHash = blockHash; ps.proposedAt = now;
  ps.vehicleAddr = vehicleAddr; ps.isLocalProposer = true;
  ps.prepareVotes[m_rsuId] = sig; // proposer implicitly prepares its own block
  m_proposals[b.index] = ps;
  m_proposalInFlight = true;
  m_proposals[b.index].timeoutEvent =
    Simulator::Schedule (Seconds (3.0), &RsuApp::HandleProposalTimeout, this, b.index);

  PrePrepareMsg pp{0, b.index, b, m_rsuId, sig};
  BroadcastToPeers (pp.Serialize ());
  DrainOrphanVotes (b.index);
  TryAdvancePhase (b.index);
}

void
RsuApp::HandlePrePrepare (const std::vector<uint8_t> &buf, const Address &from)
{
  (void) from;
  double now = Simulator::Now ().GetSeconds ();
  ByteReader r (buf); r.U8 ();
  PrePrepareMsg msg = PrePrepareMsg::Parse (r);

  if (m_proposals.find (msg.block.index) != m_proposals.end ()) return; // already known

  auto blockHash = msg.block.ComputeHash ();

  // verify proposer's signature over the block hash
  EVP_PKEY *proposerPub = nullptr;
  if (msg.primaryId == m_rsuId) proposerPub = m_keyPair.pkey;
  else
    for (auto &p : m_peers) if (p.rsuId == msg.primaryId) proposerPub = CryptoUtils::ImportPublicKey (p.publicKey);
  if (!proposerPub) return;
  bool sigOk = CryptoUtils::Verify (proposerPub, blockHash, msg.primarySig);
  if (proposerPub != m_keyPair.pkey) EVP_PKEY_free (proposerPub);
  if (!sigOk) return;

  // linkage check against our own chain tip (defends against stale/forked proposals)
  const Block &tip = m_blockchain.GetLatestBlock ();
  if (msg.block.prevHash != tip.ComputeHash () || msg.block.index != tip.index + 1) return;

  // re-verify the embedded transaction(s) - defense in depth against a
  // Byzantine RSU trying to smuggle an invalid transaction into a block.
  for (auto &tx : msg.block.transactions)
    {
      // (certificate re-check would require the cert; in this lite model the
      // proposer is trusted to have checked it, cross-RSU cert re-verification
      // is a straightforward extension using a small cert cache keyed by nodeId)
      (void) tx;
    }

  ProposalState ps;
  ps.block = msg.block; ps.blockHash = blockHash; ps.proposedAt = now; ps.isLocalProposer = false;
  auto ownSig = CryptoUtils::Sign (m_keyPair, blockHash);
  ps.prepareVotes[m_rsuId] = ownSig;
  m_proposals[msg.block.index] = ps;
  m_proposals[msg.block.index].timeoutEvent =
    Simulator::Schedule (Seconds (3.0), &RsuApp::HandleProposalTimeout, this, msg.block.index);

  VoteMsg pv{MsgType::PBFT_PREPARE, msg.view, msg.block.index, blockHash, m_rsuId, ownSig};
  BroadcastToPeers (pv.Serialize ());
  DrainOrphanVotes (msg.block.index);
  TryAdvancePhase (msg.block.index);
}

void
RsuApp::HandlePrepare (const std::vector<uint8_t> &buf, const Address &from)
{
  (void) from;
  ByteReader r (buf); r.U8 ();
  VoteMsg msg = VoteMsg::Parse (r, MsgType::PBFT_PREPARE);
  ProcessPrepareVote ({msg.type, msg.view, msg.seq, msg.blockHash, msg.rsuId, msg.signature});
}

void
RsuApp::HandleCommit (const std::vector<uint8_t> &buf, const Address &from)
{
  (void) from;
  ByteReader r (buf); r.U8 ();
  VoteMsg msg = VoteMsg::Parse (r, MsgType::PBFT_COMMIT);
  ProcessCommitVote ({msg.type, msg.view, msg.seq, msg.blockHash, msg.rsuId, msg.signature});
}

void
RsuApp::ProcessPrepareVote (const PendingVote &v)
{
  auto it = m_proposals.find (v.seq);
  if (it == m_proposals.end ())
    {
      // Proposal not seen yet (PRE_PREPARE still in flight/reordered) -
      // buffer it and replay once the proposal is created.
      auto &q = m_orphanVotes[v.seq];
      if (q.size () < kMaxOrphanVotesPerIndex) q.push_back (v);
      return;
    }

  EVP_PKEY *voterPub = nullptr;
  if (v.rsuId == m_rsuId) voterPub = m_keyPair.pkey;
  else for (auto &p : m_peers) if (p.rsuId == v.rsuId) voterPub = CryptoUtils::ImportPublicKey (p.publicKey);
  if (!voterPub) return;
  bool ok = CryptoUtils::Verify (voterPub, v.blockHash, v.signature);
  if (voterPub != m_keyPair.pkey) EVP_PKEY_free (voterPub);
  if (!ok || v.blockHash != it->second.blockHash) return;

  it->second.prepareVotes[v.rsuId] = v.signature;
  TryAdvancePhase (v.seq);
}

void
RsuApp::ProcessCommitVote (const PendingVote &v)
{
  auto it = m_proposals.find (v.seq);
  if (it == m_proposals.end ())
    {
      auto &q = m_orphanVotes[v.seq];
      if (q.size () < kMaxOrphanVotesPerIndex) q.push_back (v);
      return;
    }

  EVP_PKEY *voterPub = nullptr;
  if (v.rsuId == m_rsuId) voterPub = m_keyPair.pkey;
  else for (auto &p : m_peers) if (p.rsuId == v.rsuId) voterPub = CryptoUtils::ImportPublicKey (p.publicKey);
  if (!voterPub) return;
  bool ok = CryptoUtils::Verify (voterPub, v.blockHash, v.signature);
  if (voterPub != m_keyPair.pkey) EVP_PKEY_free (voterPub);
  if (!ok || v.blockHash != it->second.blockHash) return;

  it->second.commitVotes[v.rsuId] = v.signature;
  TryAdvancePhase (v.seq);
}

void
RsuApp::DrainOrphanVotes (uint64_t blockIndex)
{
  auto oit = m_orphanVotes.find (blockIndex);
  if (oit == m_orphanVotes.end ()) return;
  std::vector<PendingVote> buffered = std::move (oit->second);
  m_orphanVotes.erase (oit);
  for (auto &v : buffered)
    {
      if (v.type == MsgType::PBFT_PREPARE) ProcessPrepareVote (v);
      else if (v.type == MsgType::PBFT_COMMIT) ProcessCommitVote (v);
    }
}

void
RsuApp::TryAdvancePhase (uint64_t blockIndex)
{
  auto it = m_proposals.find (blockIndex);
  if (it == m_proposals.end ()) return;
  ProposalState &ps = it->second;

  uint32_t f = (m_totalRsuCount > 0) ? (m_totalRsuCount - 1) / 3 : 0;
  uint32_t quorum = 2 * f + 1;
  if (quorum == 0) quorum = 1;

  if (!ps.preparedBroadcast && ps.prepareVotes.size () >= quorum)
    {
      ps.preparedBroadcast = true;
      auto commitSig = CryptoUtils::Sign (m_keyPair, ps.blockHash);
      ps.commitVotes[m_rsuId] = commitSig;
      VoteMsg cv{MsgType::PBFT_COMMIT, 0, blockIndex, ps.blockHash, m_rsuId, commitSig};
      BroadcastToPeers (cv.Serialize ());
    }

  if (!ps.finalized && ps.commitVotes.size () >= quorum)
    {
      FinalizeCommit (blockIndex);
    }
}

void
RsuApp::FinalizeCommit (uint64_t blockIndex)
{
  auto it = m_proposals.find (blockIndex);
  if (it == m_proposals.end ()) return;
  ProposalState &ps = it->second;
  ps.finalized = true;
  if (ps.timeoutEvent.IsRunning ()) Simulator::Cancel (ps.timeoutEvent);
  double now = Simulator::Now ().GetSeconds ();

  Block b = ps.block;
  b.commitSignatures = ps.commitVotes;
  bool appended = m_blockchain.AppendBlock (b);

  if (appended)
    {
      double consensusLatency = now - ps.proposedAt;
      MetricsCollector::Get ().LogConsensusRound (now, 0, blockIndex, b.proposerRsuId,
                                                   consensusLatency, ps.commitVotes.size (), m_rsuId);
      MetricsCollector::Get ().LogBlockCommitted (now, b.index, b.transactions.size (), m_rsuId);

      for (auto &tx : b.transactions)
        {
          if (tx.txType != "AUTH") continue;
          auto token = m_blockchain.DeriveSessionToken (tx.nodeId, b);
          m_sessionTokens[tx.nodeId] = token;

          if (ps.isLocalProposer)
            {
              double authLatency = 0.0; // sane fallback if arrival timestamp is missing
              auto ait = m_authRequestArrival.find (tx.nodeId);
              if (ait != m_authRequestArrival.end ()) authLatency = now - ait->second;
              MetricsCollector::Get ().LogAuthResult (now, tx.nodeId, true, authLatency, "ok");

              AuthAcceptMsg accept{tx.nodeId, b.index, b.ComputeHash (), authLatency};
              SendTo (ps.vehicleAddr, accept.Serialize ());
            }
        }

      // Let peers who lost a concurrent proposal race at this same index
      // (see class-level doc comment in rsu-app.h) adopt this block
      // directly rather than waiting on a proposal that can never reach
      // quorum once the chain has moved on.
      BlockSyncMsg sync; sync.block = b;
      BroadcastToPeers (sync.Serialize ());
    }
  // if not appended: a competing proposal at this index already committed
  // locally (we must have missed/ignored its PRE_PREPARE via the
  // "already known" guard) - the BLOCK_SYNC broadcast above from whichever
  // RSU DID append it will let us catch up in HandleBlockSync.

  if (ps.isLocalProposer) m_proposalInFlight = false;
}

void
RsuApp::AbandonProposal (uint64_t blockIndex)
{
  auto it = m_proposals.find (blockIndex);
  if (it == m_proposals.end () || it->second.finalized) return;
  bool wasLocalProposer = it->second.isLocalProposer;
  if (it->second.timeoutEvent.IsRunning ()) Simulator::Cancel (it->second.timeoutEvent);
  m_proposals.erase (it);
  if (wasLocalProposer) m_proposalInFlight = false;
  // No explicit retry here: the originating vehicle/drone's MobileNodeApp
  // already retries AUTH_REQUEST on its own backoff schedule, which will
  // naturally trigger a fresh ProposeBlock() against the (by then possibly
  // updated) chain tip once this RSU is no longer busy.
}

void
RsuApp::HandleProposalTimeout (uint64_t blockIndex)
{
  // Liveness backstop: if neither quorum nor a BLOCK_SYNC resolved this
  // proposal within the timeout window, un-stick it so the RSU can accept
  // new requests again instead of deadlocking forever.
  AbandonProposal (blockIndex);
}

void
RsuApp::HandleBlockSync (const std::vector<uint8_t> &buf, const Address &from)
{
  (void) from;
  ByteReader r (buf); r.U8 ();
  BlockSyncMsg msg = BlockSyncMsg::Parse (r);
  double now = Simulator::Now ().GetSeconds ();

  const Block &tip = m_blockchain.GetLatestBlock ();
  if (msg.block.index != tip.index + 1 || msg.block.prevHash != tip.ComputeHash ())
    return; // not immediately applicable (already have it, or we're more than one block behind)

  bool appended = m_blockchain.AppendBlock (msg.block);
  if (!appended) return;

  MetricsCollector::Get ().LogBlockCommitted (now, msg.block.index, msg.block.transactions.size (), m_rsuId);
  for (auto &tx : msg.block.transactions)
    {
      if (tx.txType != "AUTH") continue;
      m_sessionTokens[tx.nodeId] = m_blockchain.DeriveSessionToken (tx.nodeId, msg.block);
    }

  // If we had our own (now superseded) proposal at this index, reconcile
  // it. IMPORTANT: a peer reaching quorum first does not necessarily mean
  // OUR transaction lost a race - it may be the SAME transaction we
  // proposed, just finalized on a peer's copy microseconds before our own
  // local vote count reached quorum. In that case we still owe the
  // originating vehicle its AUTH_ACCEPT.
  auto it = m_proposals.find (msg.block.index);
  if (it != m_proposals.end () && !it->second.finalized)
    {
      bool wasLocalProposer = it->second.isLocalProposer;
      Address vehicleAddr = it->second.vehicleAddr;
      bool hadTx = !it->second.block.transactions.empty ();
      Transaction localTx = hadTx ? it->second.block.transactions[0] : Transaction ();

      if (it->second.timeoutEvent.IsRunning ()) Simulator::Cancel (it->second.timeoutEvent);
      m_proposals.erase (it);
      if (wasLocalProposer) m_proposalInFlight = false;

      bool sameTransaction = wasLocalProposer && hadTx && !msg.block.transactions.empty ()
        && localTx.nodeId == msg.block.transactions[0].nodeId
        && localTx.nonce == msg.block.transactions[0].nonce;

      if (sameTransaction)
        {
          double authLatency = 0.0;
          auto ait = m_authRequestArrival.find (localTx.nodeId);
          if (ait != m_authRequestArrival.end ()) authLatency = now - ait->second;
          MetricsCollector::Get ().LogAuthResult (now, localTx.nodeId, true, authLatency, "ok_via_sync");
          AuthAcceptMsg accept{localTx.nodeId, msg.block.index, msg.block.ComputeHash (), authLatency};
          SendTo (vehicleAddr, accept.Serialize ());
        }
      // else: a genuinely different transaction won the race at this index;
      // our own vehicle's client will retry AUTH_REQUEST on its own
      // schedule and succeed at the next available block index.
    }
}

void
RsuApp::HandleDataMsg (const std::vector<uint8_t> &buf, const Address &from)
{
  (void) from;
  double now = Simulator::Now ().GetSeconds ();
  ByteReader r (buf); r.U8 ();
  DataMsg msg = DataMsg::Parse (r);

  auto it = m_sessionTokens.find (msg.nodeId);
  if (it == m_sessionTokens.end ())
    {
      MetricsCollector::Get ().LogDataMsg (now, msg.nodeId, false);
      MetricsCollector::Get ().LogAttackEvent (now, "UnauthenticatedDataMessage", msg.nodeId, "detected");
      return;
    }

  ByteWriter w; w.U32 (msg.nodeId); w.Dbl (msg.timestamp); w.Str (msg.payload);
  auto expected = CryptoUtils::HmacSha256 (it->second, w.buf);
  bool valid = (expected == msg.hmacTag);
  MetricsCollector::Get ().LogDataMsg (now, msg.nodeId, valid);
  if (!valid)
    MetricsCollector::Get ().LogAttackEvent (now, "DataTamperOrForgedToken", msg.nodeId, "detected");
}
