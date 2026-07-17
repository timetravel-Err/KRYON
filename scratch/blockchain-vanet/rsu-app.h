#ifndef RSU_APP_H
#define RSU_APP_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/address.h"
#include "ns3/event-id.h"
#include <map>
#include <vector>
#include <deque>
#include "blockchain.h"
#include "crypto-utils.h"
#include "messages.h"

// ---------------------------------------------------------------------------
// RsuApp: NS-3 Application run on every RSU node.
//
// Responsibilities:
//  1. Auth endpoint - receives AUTH_REQUEST from its assigned vehicles/
//     drones, validates CA certificate + node signature + freshness (anti-
//     replay) + per-source rate limit (anti-DoS/Sybil), then proposes a
//     block for that transaction.
//  2. PBFT-lite validator - participates in PRE_PREPARE / PREPARE / COMMIT
//     voting with peer RSUs; requires a 2f+1 quorum before appending a block
//     to its local chain (tolerates up to f Byzantine/faulty RSUs among the
//     N = 3f+1 RSU set).
//
// SIMPLIFICATION (documented for the paper's methodology/limitations
// section): rather than a single globally-elected PBFT primary ordering
// every transaction, each RSU acts as *proposer* for the transactions it
// personally authenticates ("rotating / multi-proposer PBFT-lite"). Byzantine
// safety of the 2-phase vote (prepare+commit, 2f+1 quorum) is preserved;
// what is simplified is transaction ordering/liveness under leader failure
// (no formal view-change protocol). This is a common, clearly-labelled
// simplification for first-generation VANET blockchain prototypes.
// ---------------------------------------------------------------------------
class RsuApp : public ns3::Application
{
public:
  static ns3::TypeId GetTypeId ();
  RsuApp ();
  ~RsuApp () override;

  struct PeerRsu
  {
    uint32_t rsuId;
    ns3::Address address; // InetSocketAddress
    std::vector<uint8_t> publicKey;
  };

  void Setup (uint32_t rsuId,
              CryptoUtils::KeyPair keyPair,
              std::vector<uint8_t> caPublicKeyBytes,
              std::vector<PeerRsu> peers,
              uint16_t port,
              uint32_t totalRsuCount);

  // Metrics helper for other apps to query committed session token validity.
  bool HasSessionToken (uint32_t nodeId) const;

protected:
  void DoDispose () override;

private:
  void StartApplication () override;
  void StopApplication () override;

  void HandleRead (ns3::Ptr<ns3::Socket> socket);

  void HandleAuthRequest (const std::vector<uint8_t> &buf, const ns3::Address &from);
  void HandlePrePrepare (const std::vector<uint8_t> &buf, const ns3::Address &from);
  void HandlePrepare (const std::vector<uint8_t> &buf, const ns3::Address &from);
  void HandleCommit (const std::vector<uint8_t> &buf, const ns3::Address &from);
  void HandleBlockSync (const std::vector<uint8_t> &buf, const ns3::Address &from);
  void HandleDataMsg (const std::vector<uint8_t> &buf, const ns3::Address &from);

  bool VerifyCertificate (const Certificate &cert) const;
  bool CheckRateLimit (const ns3::Address &from);
  void BroadcastToPeers (const std::vector<uint8_t> &bytes);
  void SendTo (const ns3::Address &to, const std::vector<uint8_t> &bytes);

  void ProposeBlock (const Transaction &tx, const ns3::Address &vehicleAddr);
  void TryAdvancePhase (uint64_t blockIndex);
  void FinalizeCommit (uint64_t blockIndex);
  void AbandonProposal (uint64_t blockIndex); // erase local state + unstick m_proposalInFlight
  void HandleProposalTimeout (uint64_t blockIndex); // liveness backstop
  void DrainOrphanVotes (uint64_t blockIndex);

  struct PendingVote { MsgType type; uint64_t view; uint64_t seq; std::vector<uint8_t> blockHash;
                        uint32_t rsuId; std::vector<uint8_t> signature; };
  void ProcessPrepareVote (const PendingVote &v);
  void ProcessCommitVote (const PendingVote &v);

  uint32_t m_rsuId;
  uint32_t m_totalRsuCount;
  CryptoUtils::KeyPair m_keyPair;
  EVP_PKEY *m_caPublicKey = nullptr;
  std::vector<PeerRsu> m_peers; // does not include self
  uint16_t m_port;

  ns3::Ptr<ns3::Socket> m_socket;
  Blockchain m_blockchain;

  // In-flight proposal bookkeeping, keyed by candidate block index.
  struct ProposalState
  {
    Block block;
    std::vector<uint8_t> blockHash;
    std::map<uint32_t, std::vector<uint8_t>> prepareVotes; // rsuId -> sig
    std::map<uint32_t, std::vector<uint8_t>> commitVotes;  // rsuId -> sig
    bool preparedBroadcast = false;
    bool committedBroadcast = false;
    bool finalized = false;
    double proposedAt = 0.0;
    ns3::Address vehicleAddr; // only meaningful if this RSU is the proposer
    bool isLocalProposer = false;
    ns3::EventId timeoutEvent; // liveness backstop if quorum/sync never arrives
  };
  std::map<uint64_t, ProposalState> m_proposals;
  bool m_proposalInFlight = false; // single-outstanding-proposal-per-RSU (lite model)

  // Votes that arrive before we've learned of their proposal yet (network
  // reordering of PRE_PREPARE vs. gossiped PREPARE/COMMIT). Bounded buffer,
  // drained whenever a matching proposal is created.
  std::map<uint64_t, std::vector<PendingVote>> m_orphanVotes;
  static constexpr size_t kMaxOrphanVotesPerIndex = 16;

  // node bookkeeping
  std::map<uint32_t, std::vector<uint8_t>> m_sessionTokens;   // nodeId -> HMAC key
  std::map<uint32_t, double> m_authRequestArrival;             // nodeId -> Simulator::Now at first request

  // simple sliding-window rate limiter per source address (anti-DoS / anti-Sybil-flood)
  std::map<std::string, std::deque<double>> m_recentRequestTimes;
  double m_rateWindowSec = 1.0;
  uint32_t m_rateMaxPerWindow = 5;
  double m_freshnessWindowSec = 2.0;
};

#endif // RSU_APP_H
