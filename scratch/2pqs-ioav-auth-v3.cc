#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/olsr-module.h"
#include "ns3/wifi-module.h"

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TwoPqsIoavAuthV3");

enum AttackMode : uint32_t
{
  ATTACK_NONE = 0,
  ATTACK_REPLAY = 1,
  ATTACK_STALE = 2,
  ATTACK_IMPERSONATION = 3
};

enum BlockchainMode : uint32_t
{
  BC_NONE = 0,
  BC_EMULATED_CONFIRM = 1,
  BC_ASYNC_LOG = 2
};

enum ReasonCode : uint8_t
{
  RC_OK = 0,
  RC_STALE = 1,
  RC_REPLAY = 2,
  RC_WRONG_TARGET = 3,
  RC_IMPERSONATION = 4,
  RC_TIMEOUT = 255
};

class AuthRequestHeader : public Header
{
public:
  static TypeId GetTypeId()
  {
    static TypeId tid = TypeId("ns3::AuthRequestHeader")
                            .SetParent<Header>()
                            .AddConstructor<AuthRequestHeader>();
    return tid;
  }

  TypeId GetInstanceTypeId() const override { return GetTypeId(); }

  void SetSessionId(uint32_t v) { m_sessionId = v; }
  void SetDroneId(uint32_t v) { m_droneId = v; }
  void SetClaimedDroneId(uint32_t v) { m_claimedDroneId = v; }
  void SetTargetAvId(uint32_t v) { m_targetAvId = v; }
  void SetTimestampMs(uint64_t v) { m_timestampMs = v; }
  void SetNonce(uint64_t v) { m_nonce = v; }

  uint32_t GetSessionId() const { return m_sessionId; }
  uint32_t GetDroneId() const { return m_droneId; }
  uint32_t GetClaimedDroneId() const { return m_claimedDroneId; }
  uint32_t GetTargetAvId() const { return m_targetAvId; }
  uint64_t GetTimestampMs() const { return m_timestampMs; }
  uint64_t GetNonce() const { return m_nonce; }

  uint32_t GetSerializedSize() const override
  {
    return 1 + 4 + 4 + 4 + 4 + 8 + 8;
  }

  void Serialize(Buffer::Iterator i) const override
  {
    i.WriteU8(m_msgType);
    i.WriteHtonU32(m_sessionId);
    i.WriteHtonU32(m_droneId);
    i.WriteHtonU32(m_claimedDroneId);
    i.WriteHtonU32(m_targetAvId);
    i.WriteHtonU64(m_timestampMs);
    i.WriteHtonU64(m_nonce);
  }

  uint32_t Deserialize(Buffer::Iterator i) override
  {
    m_msgType = i.ReadU8();
    m_sessionId = i.ReadNtohU32();
    m_droneId = i.ReadNtohU32();
    m_claimedDroneId = i.ReadNtohU32();
    m_targetAvId = i.ReadNtohU32();
    m_timestampMs = i.ReadNtohU64();
    m_nonce = i.ReadNtohU64();
    return GetSerializedSize();
  }

  void Print(std::ostream &os) const override
  {
    os << "REQ sid=" << m_sessionId
       << " drone=" << m_droneId
       << " claimed=" << m_claimedDroneId
       << " targetAv=" << m_targetAvId
       << " ts=" << m_timestampMs
       << " nonce=" << m_nonce;
  }

private:
  uint8_t m_msgType{1};
  uint32_t m_sessionId{0};
  uint32_t m_droneId{0};
  uint32_t m_claimedDroneId{0};
  uint32_t m_targetAvId{0};
  uint64_t m_timestampMs{0};
  uint64_t m_nonce{0};
};

class AuthResponseHeader : public Header
{
public:
  static TypeId GetTypeId()
  {
    static TypeId tid = TypeId("ns3::AuthResponseHeader")
                            .SetParent<Header>()
                            .AddConstructor<AuthResponseHeader>();
    return tid;
  }

  TypeId GetInstanceTypeId() const override { return GetTypeId(); }

  void SetSessionId(uint32_t v) { m_sessionId = v; }
  void SetAvId(uint32_t v) { m_avId = v; }
  void SetDroneId(uint32_t v) { m_droneId = v; }
  void SetRequestTimestampMs(uint64_t v) { m_requestTimestampMs = v; }
  void SetResponseTimestampMs(uint64_t v) { m_responseTimestampMs = v; }
  void SetStatus(uint8_t v) { m_status = v; }
  void SetReasonCode(uint8_t v) { m_reasonCode = v; }
  void SetBlockchainConfirmed(uint8_t v) { m_blockchainConfirmed = v; }

  uint32_t GetSessionId() const { return m_sessionId; }
  uint32_t GetAvId() const { return m_avId; }
  uint32_t GetDroneId() const { return m_droneId; }
  uint64_t GetRequestTimestampMs() const { return m_requestTimestampMs; }
  uint64_t GetResponseTimestampMs() const { return m_responseTimestampMs; }
  uint8_t GetStatus() const { return m_status; }
  uint8_t GetReasonCode() const { return m_reasonCode; }
  uint8_t GetBlockchainConfirmed() const { return m_blockchainConfirmed; }

  uint32_t GetSerializedSize() const override
  {
    return 1 + 4 + 4 + 4 + 8 + 8 + 1 + 1 + 1;
  }

  void Serialize(Buffer::Iterator i) const override
  {
    i.WriteU8(m_msgType);
    i.WriteHtonU32(m_sessionId);
    i.WriteHtonU32(m_avId);
    i.WriteHtonU32(m_droneId);
    i.WriteHtonU64(m_requestTimestampMs);
    i.WriteHtonU64(m_responseTimestampMs);
    i.WriteU8(m_status);
    i.WriteU8(m_reasonCode);
    i.WriteU8(m_blockchainConfirmed);
  }

  uint32_t Deserialize(Buffer::Iterator i) override
  {
    m_msgType = i.ReadU8();
    m_sessionId = i.ReadNtohU32();
    m_avId = i.ReadNtohU32();
    m_droneId = i.ReadNtohU32();
    m_requestTimestampMs = i.ReadNtohU64();
    m_responseTimestampMs = i.ReadNtohU64();
    m_status = i.ReadU8();
    m_reasonCode = i.ReadU8();
    m_blockchainConfirmed = i.ReadU8();
    return GetSerializedSize();
  }

  void Print(std::ostream &os) const override
  {
    os << "RESP sid=" << m_sessionId
       << " av=" << m_avId
       << " drone=" << m_droneId
       << " status=" << unsigned(m_status)
       << " reason=" << unsigned(m_reasonCode)
       << " bc=" << unsigned(m_blockchainConfirmed);
  }

private:
  uint8_t m_msgType{2};
  uint32_t m_sessionId{0};
  uint32_t m_avId{0};
  uint32_t m_droneId{0};
  uint64_t m_requestTimestampMs{0};
  uint64_t m_responseTimestampMs{0};
  uint8_t m_status{0};
  uint8_t m_reasonCode{0};
  uint8_t m_blockchainConfirmed{0};
};

struct SessionLogEntry
{
  uint32_t run{0};
  uint32_t sessionId{0};
  uint32_t droneId{0};
  uint32_t claimedDroneId{0};
  uint32_t avId{0};
  uint64_t nonce{0};
  double requestTxMs{0.0};
  double responseRxMs{0.0};
  double e2eDelayMs{0.0};
  double distanceM{0.0};
  uint32_t status{0};
  uint32_t reasonCode{0};
  uint32_t attackMode{0};
  uint32_t blockchainConfirmed{0};
  bool timedOut{false};
};

struct AuthStats
{
  uint64_t requestsSent{0};
  uint64_t responsesReceived{0};
  uint64_t authSuccess{0};
  uint64_t authRejected{0};
  uint64_t authTimeout{0};
  uint64_t blockchainConfirmedSuccess{0};
  double totalAuthE2EDelayMs{0.0};
};

class DroneAuthApp : public Application
{
public:
  struct PeerInfo
  {
    uint32_t avId;
    Ipv4Address avIp;
    uint16_t avPort;
    Ptr<Node> avNode;
  };

  void Setup(Ptr<Socket> socket,
             uint16_t localResponsePort,
             Time interval,
             uint32_t requestPacketSize,
             Time requestTimeout,
             Time localCryptoDelay,
             uint32_t run,
             uint32_t attackMode,
             double attackProbability,
             AuthStats *stats,
             std::vector<SessionLogEntry> *sessionLogs)
  {
    m_socket = socket;
    m_localResponsePort = localResponsePort;
    m_interval = interval;
    m_requestPacketSize = requestPacketSize;
    m_requestTimeout = requestTimeout;
    m_localCryptoDelay = localCryptoDelay;
    m_run = run;
    m_attackMode = attackMode;
    m_attackProbability = attackProbability;
    m_stats = stats;
    m_sessionLogs = sessionLogs;
  }

  void AddPeer(uint32_t avId, Ipv4Address avIp, uint16_t avPort, Ptr<Node> avNode)
  {
    m_peers.push_back({avId, avIp, avPort, avNode});
  }

private:
  struct PendingSession
  {
    uint32_t sessionId;
    uint32_t droneId;
    uint32_t claimedDroneId;
    uint32_t avId;
    uint64_t nonce;
    uint32_t attackModeUsed;
    Time requestTxTime;
    double distanceM;
    EventId timeoutEvent;
  };

  void StartApplication() override
  {
    m_running = true;
    m_uniform = CreateObject<UniformRandomVariable>();

    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_localResponsePort);
    m_socket->Bind(local);
    m_socket->SetRecvCallback(MakeCallback(&DroneAuthApp::HandleRead, this));

    if (!m_peers.empty())
    {
      m_sendEvent = Simulator::Schedule(Seconds(0.0), &DroneAuthApp::SendNextRequest, this);
    }
  }

  void StopApplication() override
  {
    m_running = false;

    if (m_sendEvent.IsRunning())
    {
      Simulator::Cancel(m_sendEvent);
    }

    for (auto &kv : m_pending)
    {
      if (kv.second.timeoutEvent.IsRunning())
      {
        Simulator::Cancel(kv.second.timeoutEvent);
      }
    }
    m_pending.clear();

    if (m_socket)
    {
      m_socket->Close();
    }
  }

  void ScheduleNext()
  {
    if (m_running)
    {
      m_sendEvent = Simulator::Schedule(m_interval, &DroneAuthApp::SendNextRequest, this);
    }
  }

  void SendNextRequest()
  {
    if (!m_running || m_peers.empty())
    {
      return;
    }

    const PeerInfo &peer = m_peers[m_nextPeerIndex % m_peers.size()];
    m_nextPeerIndex++;

    Simulator::Schedule(m_localCryptoDelay,
                        &DroneAuthApp::DoSendRequest,
                        this,
                        peer);

    ScheduleNext();
  }

  void DoSendRequest(PeerInfo peer)
  {
    if (!m_running)
    {
      return;
    }

    uint32_t sid = ++m_sessionCounter;
    uint32_t realDroneId = GetNode()->GetId();
    uint32_t claimedDroneId = realDroneId;
    uint64_t nowMs = Simulator::Now().GetMilliSeconds();
    uint64_t nonce = (static_cast<uint64_t>(realDroneId) << 32) ^
                     sid ^
                     (static_cast<uint64_t>(peer.avId) << 16);

    uint32_t attackUsed = ATTACK_NONE;
    if (m_attackMode != ATTACK_NONE && m_uniform->GetValue(0.0, 1.0) < m_attackProbability)
    {
      attackUsed = m_attackMode;
      if (m_attackMode == ATTACK_STALE)
      {
        nowMs = (nowMs > 5000) ? (nowMs - 5000) : 0;
      }
      else if (m_attackMode == ATTACK_IMPERSONATION)
      {
        claimedDroneId = realDroneId + 1000;
      }
      else if (m_attackMode == ATTACK_REPLAY && !m_lastNonces.empty())
      {
        nonce = m_lastNonces.back();
      }
    }

    AuthRequestHeader hdr;
    hdr.SetSessionId(sid);
    hdr.SetDroneId(realDroneId);
    hdr.SetClaimedDroneId(claimedDroneId);
    hdr.SetTargetAvId(peer.avId);
    hdr.SetTimestampMs(nowMs);
    hdr.SetNonce(nonce);

    uint32_t hdrSize = hdr.GetSerializedSize();
    uint32_t payloadSize = (m_requestPacketSize > hdrSize) ? (m_requestPacketSize - hdrSize) : 0;
    Ptr<Packet> p = Create<Packet>(payloadSize);
    p->AddHeader(hdr);

    Vector dpos = GetNode()->GetObject<MobilityModel>()->GetPosition();
    Vector apos = peer.avNode->GetObject<MobilityModel>()->GetPosition();
    double distance = CalculateDistance(dpos, apos);

    int sent = m_socket->SendTo(p, 0, InetSocketAddress(peer.avIp, peer.avPort));
    if (sent >= 0)
    {
      m_stats->requestsSent++;

      PendingSession ps;
      ps.sessionId = sid;
      ps.droneId = realDroneId;
      ps.claimedDroneId = claimedDroneId;
      ps.avId = peer.avId;
      ps.nonce = nonce;
      ps.attackModeUsed = attackUsed;
      ps.requestTxTime = Simulator::Now();
      ps.distanceM = distance;
      ps.timeoutEvent = Simulator::Schedule(m_requestTimeout, &DroneAuthApp::HandleTimeout, this, sid);
      m_pending[sid] = ps;

      m_lastNonces.push_back(nonce);
      if (m_lastNonces.size() > 16)
      {
        m_lastNonces.erase(m_lastNonces.begin());
      }
    }
  }

  void HandleTimeout(uint32_t sessionId)
  {
    auto it = m_pending.find(sessionId);
    if (it == m_pending.end())
    {
      return;
    }

    m_stats->authTimeout++;

    SessionLogEntry log;
    log.run = m_run;
    log.sessionId = it->second.sessionId;
    log.droneId = it->second.droneId;
    log.claimedDroneId = it->second.claimedDroneId;
    log.avId = it->second.avId;
    log.nonce = it->second.nonce;
    log.requestTxMs = it->second.requestTxTime.GetSeconds() * 1000.0;
    log.responseRxMs = -1.0;
    log.e2eDelayMs = -1.0;
    log.distanceM = it->second.distanceM;
    log.status = 0;
    log.reasonCode = RC_TIMEOUT;
    log.attackMode = it->second.attackModeUsed;
    log.blockchainConfirmed = 0;
    log.timedOut = true;
    m_sessionLogs->push_back(log);

    m_pending.erase(it);
  }

  void HandleRead(Ptr<Socket> socket)
  {
    Address from;
    Ptr<Packet> packet;

    while ((packet = socket->RecvFrom(from)))
    {
      AuthResponseHeader hdr;
      if (packet->GetSize() < hdr.GetSerializedSize())
      {
        continue;
      }

      packet->RemoveHeader(hdr);

      auto it = m_pending.find(hdr.GetSessionId());
      if (it == m_pending.end())
      {
        continue;
      }

      if (it->second.timeoutEvent.IsRunning())
      {
        Simulator::Cancel(it->second.timeoutEvent);
      }

      double responseRxMs = Simulator::Now().GetSeconds() * 1000.0;
      double e2eMs = (Simulator::Now() - it->second.requestTxTime).GetSeconds() * 1000.0;

      m_stats->responsesReceived++;
      if (hdr.GetStatus() == 1)
      {
        m_stats->authSuccess++;
        m_stats->totalAuthE2EDelayMs += e2eMs;
        if (hdr.GetBlockchainConfirmed() == 1)
        {
          m_stats->blockchainConfirmedSuccess++;
        }
      }
      else
      {
        m_stats->authRejected++;
      }

      SessionLogEntry log;
      log.run = m_run;
      log.sessionId = it->second.sessionId;
      log.droneId = it->second.droneId;
      log.claimedDroneId = it->second.claimedDroneId;
      log.avId = it->second.avId;
      log.nonce = it->second.nonce;
      log.requestTxMs = it->second.requestTxTime.GetSeconds() * 1000.0;
      log.responseRxMs = responseRxMs;
      log.e2eDelayMs = e2eMs;
      log.distanceM = it->second.distanceM;
      log.status = hdr.GetStatus();
      log.reasonCode = hdr.GetReasonCode();
      log.attackMode = it->second.attackModeUsed;
      log.blockchainConfirmed = hdr.GetBlockchainConfirmed();
      log.timedOut = false;
      m_sessionLogs->push_back(log);

      m_pending.erase(it);
    }
  }

  Ptr<Socket> m_socket;
  uint16_t m_localResponsePort{10000};
  Time m_interval{MilliSeconds(100)};
  uint32_t m_requestPacketSize{836};
  Time m_requestTimeout{MilliSeconds(400)};
  Time m_localCryptoDelay{MilliSeconds(1)};
  uint32_t m_run{1};
  uint32_t m_attackMode{0};
  double m_attackProbability{0.0};
  AuthStats *m_stats{nullptr};
  std::vector<SessionLogEntry> *m_sessionLogs{nullptr};

  bool m_running{false};
  EventId m_sendEvent;
  uint32_t m_sessionCounter{0};
  uint32_t m_nextPeerIndex{0};
  std::vector<PeerInfo> m_peers;
  std::map<uint32_t, PendingSession> m_pending;
  Ptr<UniformRandomVariable> m_uniform;
  std::vector<uint64_t> m_lastNonces;
};

class AvAuthApp : public Application
{
public:
  void Setup(Ptr<Socket> socket,
             uint16_t requestListenPort,
             uint32_t responsePacketSize,
             Time freshnessWindow,
             Time verifyDelay,
             uint32_t blockchainMode,
             Time blockchainCommitDelay,
             std::string blockchainLogFile)
  {
    m_socket = socket;
    m_requestListenPort = requestListenPort;
    m_responsePacketSize = responsePacketSize;
    m_freshnessWindow = freshnessWindow;
    m_verifyDelay = verifyDelay;
    m_blockchainMode = blockchainMode;
    m_blockchainCommitDelay = blockchainCommitDelay;
    m_blockchainLogFile = blockchainLogFile;
  }

private:
  struct ResponseContext
  {
    Ipv4Address dstIp;
    uint16_t dstPort;
    uint32_t sessionId;
    uint32_t avId;
    uint32_t droneId;
    uint64_t reqTsMs;
    uint8_t status;
    uint8_t reasonCode;
    uint8_t blockchainConfirmed;
  };

  void StartApplication() override
  {
    m_running = true;
    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_requestListenPort);
    m_socket->Bind(local);
    m_socket->SetRecvCallback(MakeCallback(&AvAuthApp::HandleRead, this));
  }

  void StopApplication() override
  {
    m_running = false;
    if (m_socket)
    {
      m_socket->Close();
    }
  }

  void HandleRead(Ptr<Socket> socket)
  {
    Address from;
    Ptr<Packet> packet;

    while ((packet = socket->RecvFrom(from)))
    {
      AuthRequestHeader hdr;
      if (packet->GetSize() < hdr.GetSerializedSize())
      {
        continue;
      }
      packet->RemoveHeader(hdr);

      uint8_t status = 1;
      uint8_t reasonCode = RC_OK;

      int64_t ageMs = std::llabs(static_cast<long long>(Simulator::Now().GetMilliSeconds()) -
                                 static_cast<long long>(hdr.GetTimestampMs()));

      if (hdr.GetTargetAvId() != GetNode()->GetId())
      {
        status = 0;
        reasonCode = RC_WRONG_TARGET;
      }
      else if (ageMs > m_freshnessWindow.GetMilliSeconds())
      {
        status = 0;
        reasonCode = RC_STALE;
      }
      else if (hdr.GetClaimedDroneId() != hdr.GetDroneId())
      {
        status = 0;
        reasonCode = RC_IMPERSONATION;
      }
      else if (!m_seenNonces.insert(hdr.GetNonce()).second)
      {
        status = 0;
        reasonCode = RC_REPLAY;
      }

      InetSocketAddress src = InetSocketAddress::ConvertFrom(from);

      ResponseContext ctx;
      ctx.dstIp = src.GetIpv4();
      ctx.dstPort = src.GetPort();
      ctx.sessionId = hdr.GetSessionId();
      ctx.avId = GetNode()->GetId();
      ctx.droneId = hdr.GetDroneId();
      ctx.reqTsMs = hdr.GetTimestampMs();
      ctx.status = status;
      ctx.reasonCode = reasonCode;
      ctx.blockchainConfirmed = 0;

      if (status == 1 && m_blockchainMode == BC_EMULATED_CONFIRM)
      {
        Simulator::Schedule(m_verifyDelay + m_blockchainCommitDelay,
                            &AvAuthApp::SendResponse,
                            this,
                            ctx,
                            true);
      }
      else
      {
        if (status == 1 && m_blockchainMode == BC_ASYNC_LOG)
        {
          AppendBlockchainEvent(ctx);
        }
        Simulator::Schedule(m_verifyDelay,
                            &AvAuthApp::SendResponse,
                            this,
                            ctx,
                            false);
      }
    }
  }

  void SendResponse(ResponseContext ctx, bool blockchainConfirmed)
  {
    if (!m_running)
    {
      return;
    }

    AuthResponseHeader hdr;
    hdr.SetSessionId(ctx.sessionId);
    hdr.SetAvId(ctx.avId);
    hdr.SetDroneId(ctx.droneId);
    hdr.SetRequestTimestampMs(ctx.reqTsMs);
    hdr.SetResponseTimestampMs(Simulator::Now().GetMilliSeconds());
    hdr.SetStatus(ctx.status);
    hdr.SetReasonCode(ctx.reasonCode);
    hdr.SetBlockchainConfirmed(blockchainConfirmed ? 1 : 0);

    if (ctx.status == 1 && blockchainConfirmed)
    {
      AppendBlockchainEvent(ctx);
    }

    uint32_t hdrSize = hdr.GetSerializedSize();
    uint32_t payloadSize = (m_responsePacketSize > hdrSize) ? (m_responsePacketSize - hdrSize) : 0;
    Ptr<Packet> p = Create<Packet>(payloadSize);
    p->AddHeader(hdr);

    m_socket->SendTo(p, 0, InetSocketAddress(ctx.dstIp, ctx.dstPort));
  }

  void AppendBlockchainEvent(const ResponseContext &ctx)
  {
    if (m_blockchainLogFile.empty())
    {
      return;
    }

    std::filesystem::path path(m_blockchainLogFile);
    if (path.has_parent_path())
    {
      std::filesystem::create_directories(path.parent_path());
    }

    bool writeHeader = !std::filesystem::exists(path);
    std::ofstream out(m_blockchainLogFile, std::ios::app);

    if (writeHeader)
    {
      out << "SimTimeMs,SessionId,AvId,DroneId,ReqTimestampMs,Status,ReasonCode\n";
    }

    out << Simulator::Now().GetMilliSeconds() << ","
        << ctx.sessionId << ","
        << ctx.avId << ","
        << ctx.droneId << ","
        << ctx.reqTsMs << ","
        << unsigned(ctx.status) << ","
        << unsigned(ctx.reasonCode) << "\n";
  }

  Ptr<Socket> m_socket;
  bool m_running{false};
  uint16_t m_requestListenPort{9000};
  uint32_t m_responsePacketSize{68};
  Time m_freshnessWindow{MilliSeconds(500)};
  Time m_verifyDelay{MilliSeconds(2)};
  uint32_t m_blockchainMode{0};
  Time m_blockchainCommitDelay{MilliSeconds(20)};
  std::string m_blockchainLogFile;
  std::set<uint64_t> m_seenNonces;
};

static void WriteSessionCsv(const std::string &filePath, const std::vector<SessionLogEntry> &logs)
{
  std::filesystem::path outPath(filePath);
  if (outPath.has_parent_path())
  {
    std::filesystem::create_directories(outPath.parent_path());
  }

  bool writeHeader = !std::filesystem::exists(outPath);
  std::ofstream out(filePath, std::ios::app);

  if (writeHeader)
  {
    out << "Run,SessionId,DroneId,ClaimedDroneId,AvId,Nonce,RequestTxMs,ResponseRxMs,E2EDelayMs,DistanceM,Status,ReasonCode,AttackMode,BlockchainConfirmed,TimedOut\n";
  }

  for (const auto &log : logs)
  {
    out << log.run << ","
        << log.sessionId << ","
        << log.droneId << ","
        << log.claimedDroneId << ","
        << log.avId << ","
        << log.nonce << ","
        << std::fixed << std::setprecision(6)
        << log.requestTxMs << ","
        << log.responseRxMs << ","
        << log.e2eDelayMs << ","
        << log.distanceM << ","
        << log.status << ","
        << log.reasonCode << ","
        << log.attackMode << ","
        << log.blockchainConfirmed << ","
        << (log.timedOut ? 1 : 0) << "\n";
  }
}

int main(int argc, char *argv[])
{
  uint32_t numRegions = 1;
  uint32_t dronesPerRegion = 4;
  uint32_t avsPerRegion = 8;
  uint32_t run = 1;
  double simTime = 30.0;
  double appStopMargin = 2.0;
  double authIntervalMs = 100.0;
  double authTimeoutMs = 400.0;
  double freshnessMs = 500.0;
  double droneCryptoDelayMs = 1.0;
  double avVerifyDelayMs = 2.0;
  uint32_t attackMode = ATTACK_NONE;
  double attackProbability = 0.0;
  uint32_t blockchainMode = BC_NONE;
  double blockchainCommitDelayMs = 20.0;

  std::string summaryCsv = "experiments/results-auth-v3-summary.csv";
  std::string sessionsCsv = "experiments/results-auth-v3-sessions.csv";
  std::string flowmonXml = "experiments/results-auth-v3-flowmon.xml";
  std::string blockchainCsv = "experiments/results-auth-v3-blockchain.csv";

  CommandLine cmd;
  cmd.AddValue("numRegions", "Number of regions", numRegions);
  cmd.AddValue("dronesPerRegion", "Drones per region", dronesPerRegion);
  cmd.AddValue("avsPerRegion", "AVs per region", avsPerRegion);
  cmd.AddValue("run", "Run index", run);
  cmd.AddValue("simTime", "Simulation time", simTime);
  cmd.AddValue("appStopMargin", "Stop apps before end", appStopMargin);
  cmd.AddValue("authIntervalMs", "Authentication interval in ms", authIntervalMs);
  cmd.AddValue("authTimeoutMs", "Authentication timeout in ms", authTimeoutMs);
  cmd.AddValue("freshnessMs", "Freshness window in ms", freshnessMs);
  cmd.AddValue("droneCryptoDelayMs", "Drone processing delay in ms", droneCryptoDelayMs);
  cmd.AddValue("avVerifyDelayMs", "AV verification delay in ms", avVerifyDelayMs);
  cmd.AddValue("attackMode", "0 none, 1 replay, 2 stale, 3 impersonation", attackMode);
  cmd.AddValue("attackProbability", "Probability of attack injection [0,1]", attackProbability);
  cmd.AddValue("blockchainMode", "0 none, 1 emulated confirm, 2 async log", blockchainMode);
  cmd.AddValue("blockchainCommitDelayMs", "Blockchain confirmation delay in ms", blockchainCommitDelayMs);
  cmd.AddValue("summaryCsv", "Summary CSV", summaryCsv);
  cmd.AddValue("sessionsCsv", "Sessions CSV", sessionsCsv);
  cmd.AddValue("flowmonXml", "FlowMonitor XML", flowmonXml);
  cmd.AddValue("blockchainCsv", "Blockchain event CSV", blockchainCsv);
  cmd.Parse(argc, argv);

  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(run);

  double appStopTime = simTime - appStopMargin;
  uint32_t totalDrones = numRegions * dronesPerRegion;
  uint32_t totalAVs = numRegions * avsPerRegion;
  uint32_t totalNodes = totalDrones + totalAVs;

  NodeContainer drones;
  drones.Create(totalDrones);

  NodeContainer avs;
  avs.Create(totalAVs);

  OlsrHelper olsr;
  Ipv4StaticRoutingHelper staticRouting;
  Ipv4ListRoutingHelper routing;
  routing.Add(staticRouting, 0);
  routing.Add(olsr, 10);

  InternetStackHelper internet;
  internet.SetRoutingHelper(routing);
  internet.Install(drones);
  internet.Install(avs);

  double droneAltitudeMin = 50.0;
  double droneAltitudeMax = 150.0;
  double regionRadius = std::max(100.0, 50.0 * std::sqrt(static_cast<double>(totalNodes)));

  for (uint32_t r = 0; r < numRegions; ++r)
  {
    double regionCenterX = 500.0 * r;
    double regionCenterY = 0.0;

    for (uint32_t d = 0; d < dronesPerRegion; ++d)
    {
      uint32_t droneIdx = r * dronesPerRegion + d;
      MobilityHelper mob;
      Ptr<RandomBoxPositionAllocator> posAlloc = CreateObject<RandomBoxPositionAllocator>();
      posAlloc->SetAttribute("X",
                             StringValue("ns3::UniformRandomVariable[Min=" +
                                         std::to_string(regionCenterX - regionRadius) + "|Max=" +
                                         std::to_string(regionCenterX + regionRadius) + "]"));
      posAlloc->SetAttribute("Y",
                             StringValue("ns3::UniformRandomVariable[Min=" +
                                         std::to_string(regionCenterY - regionRadius) + "|Max=" +
                                         std::to_string(regionCenterY + regionRadius) + "]"));
      posAlloc->SetAttribute("Z",
                             StringValue("ns3::UniformRandomVariable[Min=" +
                                         std::to_string(droneAltitudeMin) + "|Max=" +
                                         std::to_string(droneAltitudeMax) + "]"));
      mob.SetPositionAllocator(posAlloc);
      mob.SetMobilityModel("ns3::RandomWaypointMobilityModel",
                           "Speed", StringValue("ns3::UniformRandomVariable[Min=5|Max=15]"),
                           "Pause", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"),
                           "PositionAllocator", PointerValue(posAlloc));
      mob.Install(drones.Get(droneIdx));
    }

    for (uint32_t a = 0; a < avsPerRegion; ++a)
    {
      uint32_t avIdx = r * avsPerRegion + a;
      MobilityHelper mob;
      Ptr<RandomBoxPositionAllocator> posAlloc = CreateObject<RandomBoxPositionAllocator>();
      posAlloc->SetAttribute("X",
                             StringValue("ns3::UniformRandomVariable[Min=" +
                                         std::to_string(regionCenterX - regionRadius) + "|Max=" +
                                         std::to_string(regionCenterX + regionRadius) + "]"));
      posAlloc->SetAttribute("Y",
                             StringValue("ns3::UniformRandomVariable[Min=" +
                                         std::to_string(regionCenterY - regionRadius) + "|Max=" +
                                         std::to_string(regionCenterY + regionRadius) + "]"));
      posAlloc->SetAttribute("Z",
                             StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
      mob.SetPositionAllocator(posAlloc);
      mob.SetMobilityModel("ns3::RandomWaypointMobilityModel",
                           "Speed", StringValue("ns3::UniformRandomVariable[Min=8|Max=17]"),
                           "Pause", StringValue("ns3::ConstantRandomVariable[Constant=0.5]"),
                           "PositionAllocator", PointerValue(posAlloc));
      mob.Install(avs.Get(avIdx));
    }
  }

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211n);
  wifi.SetRemoteStationManager("ns3::MinstrelHtWifiManager",
                               "RtsCtsThreshold", UintegerValue(100));

  WifiMacHelper wifiMac;
  wifiMac.SetType("ns3::AdhocWifiMac");

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel",
                                 "Frequency", DoubleValue(5.0e9));

  YansWifiPhyHelper wifiPhy;
  wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
  wifiPhy.Set("TxPowerStart", DoubleValue(23.0));
  wifiPhy.Set("TxPowerEnd", DoubleValue(23.0));
  wifiPhy.Set("RxSensitivity", DoubleValue(-82.0));
  wifiPhy.SetChannel(wifiChannel.Create());

  NetDeviceContainer droneDevices = wifi.Install(wifiPhy, wifiMac, drones);
  NetDeviceContainer avDevices = wifi.Install(wifiPhy, wifiMac, avs);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.0.0", "255.255.0.0");
  Ipv4InterfaceContainer droneInterfaces = ipv4.Assign(droneDevices);
  Ipv4InterfaceContainer avInterfaces = ipv4.Assign(avDevices);

  uint16_t requestPort = 9000;
  uint16_t responseBasePort = 10000;

  std::vector<AuthStats> droneStats(totalDrones);
  std::vector<std::vector<SessionLogEntry>> allSessionLogs(totalDrones);
  std::vector<Ptr<DroneAuthApp>> droneApps(totalDrones);

  for (uint32_t a = 0; a < totalAVs; ++a)
  {
    Ptr<Socket> avSocket = Socket::CreateSocket(avs.Get(a), UdpSocketFactory::GetTypeId());
    Ptr<AvAuthApp> avApp = CreateObject<AvAuthApp>();
    avApp->Setup(avSocket,
                 requestPort,
                 68,
                 MilliSeconds(freshnessMs),
                 MilliSeconds(avVerifyDelayMs),
                 blockchainMode,
                 MilliSeconds(blockchainCommitDelayMs),
                 blockchainCsv);
    avs.Get(a)->AddApplication(avApp);
    avApp->SetStartTime(Seconds(0.0));
    avApp->SetStopTime(Seconds(appStopTime));
  }

  for (uint32_t d = 0; d < totalDrones; ++d)
  {
    Ptr<Socket> droneSocket = Socket::CreateSocket(drones.Get(d), UdpSocketFactory::GetTypeId());
    Ptr<DroneAuthApp> droneApp = CreateObject<DroneAuthApp>();
    droneApp->Setup(droneSocket,
                    responseBasePort + d,
                    MilliSeconds(authIntervalMs),
                    836,
                    MilliSeconds(authTimeoutMs),
                    MilliSeconds(droneCryptoDelayMs),
                    run,
                    attackMode,
                    attackProbability,
                    &droneStats[d],
                    &allSessionLogs[d]);
    drones.Get(d)->AddApplication(droneApp);
    droneApp->SetStartTime(Seconds(10.0));
    droneApp->SetStopTime(Seconds(appStopTime));
    droneApps[d] = droneApp;
  }

  for (uint32_t a = 0; a < totalAVs; ++a)
  {
    uint32_t targetDrone = a % totalDrones;
    droneApps[targetDrone]->AddPeer(avs.Get(a)->GetId(),
                                    avInterfaces.GetAddress(a),
                                    requestPort,
                                    avs.Get(a));
  }

  FlowMonitorHelper flowHelper;
  Ptr<FlowMonitor> monitor = flowHelper.InstallAll();

  Simulator::Stop(Seconds(simTime));
  Simulator::Run();

  monitor->CheckForLostPackets();
  monitor->SerializeToXmlFile(flowmonXml, true, true);

  double totalThroughputKbps = 0.0;
  double totalFlowDelayMs = 0.0;
  double totalFlowJitterMs = 0.0;
  uint64_t totalTxPackets = 0;
  uint64_t totalRxPackets = 0;
  uint32_t flowCount = 0;

  for (const auto &flow : monitor->GetFlowStats())
  {
    const auto &stats = flow.second;
    totalTxPackets += stats.txPackets;
    totalRxPackets += stats.rxPackets;

    if (stats.rxPackets > 0)
    {
      double duration = stats.timeLastRxPacket.GetSeconds() - stats.timeFirstTxPacket.GetSeconds();
      if (duration > 0)
      {
        totalThroughputKbps += (stats.rxBytes * 8.0) / (duration * 1e3);
      }
      totalFlowDelayMs += (stats.delaySum.GetSeconds() * 1000.0) / stats.rxPackets;
      totalFlowJitterMs += (stats.jitterSum.GetSeconds() * 1000.0) / stats.rxPackets;
      flowCount++;
    }
  }

  double avgThroughputKbps = (flowCount > 0) ? totalThroughputKbps / flowCount : 0.0;
  double avgFlowDelayMs = (flowCount > 0) ? totalFlowDelayMs / flowCount : 0.0;
  double avgFlowJitterMs = (flowCount > 0) ? totalFlowJitterMs / flowCount : 0.0;
  double pdr = (totalTxPackets > 0) ? static_cast<double>(totalRxPackets) / totalTxPackets : 0.0;

  uint64_t totalReq = 0, totalResp = 0, totalSucc = 0, totalReject = 0, totalTimeout = 0, totalBcConfirmed = 0;
  double totalAuthE2E = 0.0;
  std::vector<SessionLogEntry> mergedLogs;

  for (uint32_t d = 0; d < totalDrones; ++d)
  {
    totalReq += droneStats[d].requestsSent;
    totalResp += droneStats[d].responsesReceived;
    totalSucc += droneStats[d].authSuccess;
    totalReject += droneStats[d].authRejected;
    totalTimeout += droneStats[d].authTimeout;
    totalBcConfirmed += droneStats[d].blockchainConfirmedSuccess;
    totalAuthE2E += droneStats[d].totalAuthE2EDelayMs;
    mergedLogs.insert(mergedLogs.end(), allSessionLogs[d].begin(), allSessionLogs[d].end());
  }

  double authSuccessRate = (totalReq > 0) ? static_cast<double>(totalSucc) / totalReq : 0.0;
  double avgAuthE2EDelayMs = (totalSucc > 0) ? totalAuthE2E / totalSucc : 0.0;

  std::filesystem::path summaryPath(summaryCsv);
  if (summaryPath.has_parent_path())
  {
    std::filesystem::create_directories(summaryPath.parent_path());
  }

  bool writeHeader = !std::filesystem::exists(summaryPath);
  std::ofstream out(summaryCsv, std::ios::app);

  if (writeHeader)
  {
    out << "Run,Regions,DronesPerRegion,AVsPerRegion,SimTime,AppStopTime,AuthIntervalMs,AuthTimeoutMs,FreshnessMs,DroneCryptoDelayMs,AvVerifyDelayMs,AttackMode,AttackProbability,BlockchainMode,BlockchainCommitDelayMs,ThroughputKbps,FlowDelayMs,FlowJitterMs,PDR,AuthRequests,AuthResponses,AuthSuccess,AuthRejected,AuthTimeout,BlockchainConfirmedSuccess,AuthSuccessRate,AuthAvgE2EDelayMs\n";
  }

  out << run << ","
      << numRegions << ","
      << dronesPerRegion << ","
      << avsPerRegion << ","
      << simTime << ","
      << appStopTime << ","
      << authIntervalMs << ","
      << authTimeoutMs << ","
      << freshnessMs << ","
      << droneCryptoDelayMs << ","
      << avVerifyDelayMs << ","
      << attackMode << ","
      << attackProbability << ","
      << blockchainMode << ","
      << blockchainCommitDelayMs << ","
      << std::fixed << std::setprecision(6)
      << avgThroughputKbps << ","
      << avgFlowDelayMs << ","
      << avgFlowJitterMs << ","
      << pdr << ","
      << totalReq << ","
      << totalResp << ","
      << totalSucc << ","
      << totalReject << ","
      << totalTimeout << ","
      << totalBcConfirmed << ","
      << authSuccessRate << ","
      << avgAuthE2EDelayMs << "\n";

  out.close();
  WriteSessionCsv(sessionsCsv, mergedLogs);

  Simulator::Destroy();
  return 0;
}
