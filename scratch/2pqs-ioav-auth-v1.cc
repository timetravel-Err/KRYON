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
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TwoPqsIoavAuthV1");

/*
 * 2PQS-IoAV realistic baseline v1
 *
 * Features:
 * - UAV/AV ad-hoc Wi-Fi network
 * - Custom application-layer authentication packets
 * - Request/response matching via sessionId
 * - Application-level end-to-end authentication delay
 * - FlowMonitor network metrics
 * - Reproducible CSV output
 *
 * Build/run:
 *   ./ns3 run "scratch/2pqs-ioav-auth-v1 --numRegions=1 --dronesPerRegion=5 --avsPerRegion=10 --run=1"
 */

class AuthRequestHeader : public Header
{
public:
  AuthRequestHeader() = default;

  static TypeId GetTypeId()
  {
    static TypeId tid = TypeId("ns3::AuthRequestHeader")
                            .SetParent<Header>()
                            .AddConstructor<AuthRequestHeader>();
    return tid;
  }

  TypeId GetInstanceTypeId() const override
  {
    return GetTypeId();
  }

  void SetSessionId(uint32_t v) { m_sessionId = v; }
  void SetDroneId(uint32_t v) { m_droneId = v; }
  void SetTimestampMs(uint64_t v) { m_timestampMs = v; }
  void SetNonce(uint64_t v) { m_nonce = v; }

  uint32_t GetSessionId() const { return m_sessionId; }
  uint32_t GetDroneId() const { return m_droneId; }
  uint64_t GetTimestampMs() const { return m_timestampMs; }
  uint64_t GetNonce() const { return m_nonce; }

  uint32_t GetSerializedSize() const override
  {
    return 1 + 4 + 4 + 8 + 8;
  }

  void Serialize(Buffer::Iterator start) const override
  {
    start.WriteU8(m_msgType);
    start.WriteHtonU32(m_sessionId);
    start.WriteHtonU32(m_droneId);
    start.WriteHtonU64(m_timestampMs);
    start.WriteHtonU64(m_nonce);
  }

  uint32_t Deserialize(Buffer::Iterator start) override
  {
    m_msgType = start.ReadU8();
    m_sessionId = start.ReadNtohU32();
    m_droneId = start.ReadNtohU32();
    m_timestampMs = start.ReadNtohU64();
    m_nonce = start.ReadNtohU64();
    return GetSerializedSize();
  }

  void Print(std::ostream& os) const override
  {
    os << "REQ type=" << unsigned(m_msgType)
       << " sid=" << m_sessionId
       << " drone=" << m_droneId
       << " tsMs=" << m_timestampMs
       << " nonce=" << m_nonce;
  }

private:
  uint8_t m_msgType{1};
  uint32_t m_sessionId{0};
  uint32_t m_droneId{0};
  uint64_t m_timestampMs{0};
  uint64_t m_nonce{0};
};

class AuthResponseHeader : public Header
{
public:
  AuthResponseHeader() = default;

  static TypeId GetTypeId()
  {
    static TypeId tid = TypeId("ns3::AuthResponseHeader")
                            .SetParent<Header>()
                            .AddConstructor<AuthResponseHeader>();
    return tid;
  }

  TypeId GetInstanceTypeId() const override
  {
    return GetTypeId();
  }

  void SetSessionId(uint32_t v) { m_sessionId = v; }
  void SetAvId(uint32_t v) { m_avId = v; }
  void SetRequestTimestampMs(uint64_t v) { m_requestTimestampMs = v; }
  void SetResponseTimestampMs(uint64_t v) { m_responseTimestampMs = v; }
  void SetStatus(uint8_t v) { m_status = v; }

  uint32_t GetSessionId() const { return m_sessionId; }
  uint32_t GetAvId() const { return m_avId; }
  uint64_t GetRequestTimestampMs() const { return m_requestTimestampMs; }
  uint64_t GetResponseTimestampMs() const { return m_responseTimestampMs; }
  uint8_t GetStatus() const { return m_status; }

  uint32_t GetSerializedSize() const override
  {
    return 1 + 4 + 4 + 8 + 8 + 1;
  }

  void Serialize(Buffer::Iterator start) const override
  {
    start.WriteU8(m_msgType);
    start.WriteHtonU32(m_sessionId);
    start.WriteHtonU32(m_avId);
    start.WriteHtonU64(m_requestTimestampMs);
    start.WriteHtonU64(m_responseTimestampMs);
    start.WriteU8(m_status);
  }

  uint32_t Deserialize(Buffer::Iterator start) override
  {
    m_msgType = start.ReadU8();
    m_sessionId = start.ReadNtohU32();
    m_avId = start.ReadNtohU32();
    m_requestTimestampMs = start.ReadNtohU64();
    m_responseTimestampMs = start.ReadNtohU64();
    m_status = start.ReadU8();
    return GetSerializedSize();
  }

  void Print(std::ostream& os) const override
  {
    os << "RESP type=" << unsigned(m_msgType)
       << " sid=" << m_sessionId
       << " av=" << m_avId
       << " reqTsMs=" << m_requestTimestampMs
       << " respTsMs=" << m_responseTimestampMs
       << " status=" << unsigned(m_status);
  }

private:
  uint8_t m_msgType{2};
  uint32_t m_sessionId{0};
  uint32_t m_avId{0};
  uint64_t m_requestTimestampMs{0};
  uint64_t m_responseTimestampMs{0};
  uint8_t m_status{0};
};

struct AuthStats
{
  uint64_t requestsSent{0};
  uint64_t responsesReceived{0};
  uint64_t authSuccess{0};
  uint64_t authRejected{0};
  uint64_t authTimeout{0};
  double totalAuthE2EDelayMs{0.0};
};

class DroneAuthApp : public Application
{
public:
  DroneAuthApp() = default;

  void Setup(Ptr<Socket> socket,
             Ipv4Address peerIp,
             uint16_t requestPort,
             uint16_t localResponsePort,
             Time interval,
             uint32_t requestPacketSize,
             Time timeout,
             AuthStats* stats)
  {
    m_socket = socket;
    m_peerIp = peerIp;
    m_requestPort = requestPort;
    m_localResponsePort = localResponsePort;
    m_interval = interval;
    m_requestPacketSize = requestPacketSize;
    m_timeout = timeout;
    m_stats = stats;
  }

private:
  struct PendingSession
  {
    Time sendTime;
    EventId timeoutEvent;
  };

  void StartApplication() override
  {
    m_running = true;

    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_localResponsePort);
    m_socket->Bind(local);
    m_socket->SetRecvCallback(MakeCallback(&DroneAuthApp::HandleRead, this));

    ScheduleNextRequest(Seconds(0.0));
  }

  void StopApplication() override
  {
    m_running = false;

    if (m_sendEvent.IsRunning())
    {
      Simulator::Cancel(m_sendEvent);
    }

    for (auto& kv : m_pending)
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

  void ScheduleNextRequest(Time delay)
  {
    if (m_running)
    {
      m_sendEvent = Simulator::Schedule(delay, &DroneAuthApp::SendRequest, this);
    }
  }

  void SendRequest()
  {
    if (!m_running)
    {
      return;
    }

    uint32_t sid = ++m_sessionCounter;
    uint64_t nowMs = Simulator::Now().GetMilliSeconds();
    uint64_t nonce = (static_cast<uint64_t>(GetNode()->GetId()) << 32) | sid;

    AuthRequestHeader hdr;
    hdr.SetSessionId(sid);
    hdr.SetDroneId(GetNode()->GetId());
    hdr.SetTimestampMs(nowMs);
    hdr.SetNonce(nonce);

    uint32_t hdrSize = hdr.GetSerializedSize();
    uint32_t payloadSize = (m_requestPacketSize > hdrSize) ? (m_requestPacketSize - hdrSize) : 0;

    Ptr<Packet> p = Create<Packet>(payloadSize);
    p->AddHeader(hdr);

    int sent = m_socket->SendTo(p, 0, InetSocketAddress(m_peerIp, m_requestPort));
    if (sent >= 0)
    {
      m_stats->requestsSent++;

      PendingSession ps;
      ps.sendTime = Simulator::Now();
      ps.timeoutEvent = Simulator::Schedule(m_timeout, &DroneAuthApp::HandleTimeout, this, sid);
      m_pending[sid] = ps;
    }

    ScheduleNextRequest(m_interval);
  }

  void HandleTimeout(uint32_t sid)
  {
    auto it = m_pending.find(sid);
    if (it != m_pending.end())
    {
      m_stats->authTimeout++;
      m_pending.erase(it);
    }
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

      double e2eMs = (Simulator::Now() - it->second.sendTime).GetSeconds() * 1000.0;
      m_stats->responsesReceived++;

      if (hdr.GetStatus() == 1)
      {
        m_stats->authSuccess++;
        m_stats->totalAuthE2EDelayMs += e2eMs;
      }
      else
      {
        m_stats->authRejected++;
      }

      m_pending.erase(it);
    }
  }

  Ptr<Socket> m_socket;
  Ipv4Address m_peerIp;
  uint16_t m_requestPort{9000};
  uint16_t m_localResponsePort{9001};
  Time m_interval{MilliSeconds(100)};
  uint32_t m_requestPacketSize{836};
  Time m_timeout{MilliSeconds(300)};
  EventId m_sendEvent;
  bool m_running{false};
  uint32_t m_sessionCounter{0};
  std::map<uint32_t, PendingSession> m_pending;
  AuthStats* m_stats{nullptr};
};

class AvAuthApp : public Application
{
public:
  AvAuthApp() = default;

  void Setup(Ptr<Socket> socket,
             uint16_t requestListenPort,
             uint32_t responsePacketSize,
             Time freshnessWindow,
             Time processingDelay)
  {
    m_socket = socket;
    m_requestListenPort = requestListenPort;
    m_responsePacketSize = responsePacketSize;
    m_freshnessWindow = freshnessWindow;
    m_processingDelay = processingDelay;
  }

private:
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

      int64_t ageMs = std::llabs(static_cast<long long>(Simulator::Now().GetMilliSeconds()) -
                                 static_cast<long long>(hdr.GetTimestampMs()));

      bool fresh = ageMs <= m_freshnessWindow.GetMilliSeconds();
      bool replay = !m_seenNonces.insert(hdr.GetNonce()).second;
      uint8_t status = (fresh && !replay) ? 1 : 0;

      InetSocketAddress src = InetSocketAddress::ConvertFrom(from);
      Ipv4Address dstIp = src.GetIpv4();
      uint16_t dstPort = src.GetPort();

      Simulator::Schedule(m_processingDelay,
                          &AvAuthApp::SendResponse,
                          this,
                          dstIp,
                          dstPort,
                          hdr.GetSessionId(),
                          hdr.GetTimestampMs(),
                          status);
    }
  }

  void SendResponse(Ipv4Address dstIp,
                    uint16_t dstPort,
                    uint32_t sid,
                    uint64_t reqTsMs,
                    uint8_t status)
  {
    if (!m_running)
    {
      return;
    }

    AuthResponseHeader hdr;
    hdr.SetSessionId(sid);
    hdr.SetAvId(GetNode()->GetId());
    hdr.SetRequestTimestampMs(reqTsMs);
    hdr.SetResponseTimestampMs(Simulator::Now().GetMilliSeconds());
    hdr.SetStatus(status);

    uint32_t hdrSize = hdr.GetSerializedSize();
    uint32_t payloadSize = (m_responsePacketSize > hdrSize) ? (m_responsePacketSize - hdrSize) : 0;

    Ptr<Packet> p = Create<Packet>(payloadSize);
    p->AddHeader(hdr);

    m_socket->SendTo(p, 0, InetSocketAddress(dstIp, dstPort));
  }

  Ptr<Socket> m_socket;
  bool m_running{false};
  uint16_t m_requestListenPort{9000};
  uint32_t m_responsePacketSize{68};
  Time m_freshnessWindow{MilliSeconds(500)};
  Time m_processingDelay{MilliSeconds(2)};
  std::set<uint64_t> m_seenNonces;
};

int
main(int argc, char* argv[])
{
  uint32_t numRegions = 1;
  uint32_t dronesPerRegion = 5;
  uint32_t avsPerRegion = 10;
  uint32_t run = 1;
  double simTime = 60.0;
  double appStopMargin = 2.0;
  double authIntervalMs = 100.0;
  double authTimeoutMs = 300.0;
  double freshnessMs = 500.0;
  double processingDelayMs = 2.0;
  std::string csvFile = "experiments/results-auth-v1.csv";

  CommandLine cmd;
  cmd.AddValue("numRegions", "Number of regions", numRegions);
  cmd.AddValue("dronesPerRegion", "Number of drones per region", dronesPerRegion);
  cmd.AddValue("avsPerRegion", "Number of AVs per region", avsPerRegion);
  cmd.AddValue("run", "Run index", run);
  cmd.AddValue("simTime", "Simulation time in seconds", simTime);
  cmd.AddValue("appStopMargin", "Stop applications this many seconds before simulation end", appStopMargin);
  cmd.AddValue("authIntervalMs", "Authentication request interval in ms", authIntervalMs);
  cmd.AddValue("authTimeoutMs", "Authentication timeout in ms", authTimeoutMs);
  cmd.AddValue("freshnessMs", "Freshness window in ms", freshnessMs);
  cmd.AddValue("processingDelayMs", "AV processing delay in ms", processingDelayMs);
  cmd.AddValue("csvFile", "CSV output file", csvFile);
  cmd.Parse(argc, argv);

  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(run);

  uint32_t totalDrones = numRegions * dronesPerRegion;
  uint32_t totalAVs = numRegions * avsPerRegion;
  uint32_t totalNodes = totalDrones + totalAVs;

  double appStopTime = simTime - appStopMargin;

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

      MobilityHelper droneMobility;
      Ptr<RandomBoxPositionAllocator> posAlloc = CreateObject<RandomBoxPositionAllocator>();

      posAlloc->SetAttribute("X",
                             StringValue("ns3::UniformRandomVariable[Min=" +
                                         std::to_string(regionCenterX - regionRadius) +
                                         "|Max=" +
                                         std::to_string(regionCenterX + regionRadius) + "]"));
      posAlloc->SetAttribute("Y",
                             StringValue("ns3::UniformRandomVariable[Min=" +
                                         std::to_string(regionCenterY - regionRadius) +
                                         "|Max=" +
                                         std::to_string(regionCenterY + regionRadius) + "]"));
      posAlloc->SetAttribute("Z",
                             StringValue("ns3::UniformRandomVariable[Min=" +
                                         std::to_string(droneAltitudeMin) +
                                         "|Max=" +
                                         std::to_string(droneAltitudeMax) + "]"));

      droneMobility.SetPositionAllocator(posAlloc);
      droneMobility.SetMobilityModel("ns3::RandomWaypointMobilityModel",
                                     "Speed",
                                     StringValue("ns3::UniformRandomVariable[Min=5|Max=15]"),
                                     "Pause",
                                     StringValue("ns3::ConstantRandomVariable[Constant=1.0]"),
                                     "PositionAllocator",
                                     PointerValue(posAlloc));
      droneMobility.Install(drones.Get(droneIdx));
    }

    for (uint32_t a = 0; a < avsPerRegion; ++a)
    {
      uint32_t avIdx = r * avsPerRegion + a;

      MobilityHelper avMobility;
      Ptr<RandomBoxPositionAllocator> avPosAlloc = CreateObject<RandomBoxPositionAllocator>();

      avPosAlloc->SetAttribute("X",
                               StringValue("ns3::UniformRandomVariable[Min=" +
                                           std::to_string(regionCenterX - regionRadius) +
                                           "|Max=" +
                                           std::to_string(regionCenterX + regionRadius) + "]"));
      avPosAlloc->SetAttribute("Y",
                               StringValue("ns3::UniformRandomVariable[Min=" +
                                           std::to_string(regionCenterY - regionRadius) +
                                           "|Max=" +
                                           std::to_string(regionCenterY + regionRadius) + "]"));
      avPosAlloc->SetAttribute("Z",
                               StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));

      avMobility.SetPositionAllocator(avPosAlloc);
      avMobility.SetMobilityModel("ns3::RandomWaypointMobilityModel",
                                  "Speed",
                                  StringValue("ns3::UniformRandomVariable[Min=8|Max=17]"),
                                  "Pause",
                                  StringValue("ns3::ConstantRandomVariable[Constant=0.5]"),
                                  "PositionAllocator",
                                  PointerValue(avPosAlloc));
      avMobility.Install(avs.Get(avIdx));
    }
  }

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211n);

  wifi.SetRemoteStationManager("ns3::MinstrelHtWifiManager",
                               "RtsCtsThreshold",
                               UintegerValue(100));

  WifiMacHelper wifiMac;
  wifiMac.SetType("ns3::AdhocWifiMac");

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel",
                                 "Frequency",
                                 DoubleValue(5.0e9));

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

  for (uint32_t a = 0; a < totalAVs; ++a)
  {
    Ptr<Socket> avSocket = Socket::CreateSocket(avs.Get(a), UdpSocketFactory::GetTypeId());
    Ptr<AvAuthApp> avApp = CreateObject<AvAuthApp>();
    avApp->Setup(avSocket,
                 requestPort,
                 68,
                 MilliSeconds(freshnessMs),
                 MilliSeconds(processingDelayMs));
    avs.Get(a)->AddApplication(avApp);
    avApp->SetStartTime(Seconds(0.0));
    avApp->SetStopTime(Seconds(appStopTime));
  }

  for (uint32_t a = 0; a < totalAVs; ++a)
  {
    uint32_t targetDrone = a % totalDrones;
    uint16_t localResponsePort = responseBasePort + targetDrone;

    Ptr<Socket> droneSocket = Socket::CreateSocket(drones.Get(targetDrone), UdpSocketFactory::GetTypeId());
    Ptr<DroneAuthApp> droneApp = CreateObject<DroneAuthApp>();
    droneApp->Setup(droneSocket,
                    avInterfaces.GetAddress(a),
                    requestPort,
                    localResponsePort,
                    MilliSeconds(authIntervalMs),
                    836,
                    MilliSeconds(authTimeoutMs),
                    &droneStats[targetDrone]);

    drones.Get(targetDrone)->AddApplication(droneApp);
    droneApp->SetStartTime(Seconds(10.0 + 0.1 * a));
    droneApp->SetStopTime(Seconds(appStopTime));
  }

  FlowMonitorHelper flowHelper;
  Ptr<FlowMonitor> monitor = flowHelper.InstallAll();

  Simulator::Stop(Seconds(simTime));
  Simulator::Run();

  monitor->CheckForLostPackets();

  double totalThroughput = 0.0;
  double totalDelayMs = 0.0;
  double totalJitterMs = 0.0;
  uint64_t totalTxPackets = 0;
  uint64_t totalRxPackets = 0;
  uint32_t flowCount = 0;

  for (const auto& flow : monitor->GetFlowStats())
  {
    const auto& stats = flow.second;
    totalTxPackets += stats.txPackets;
    totalRxPackets += stats.rxPackets;

    if (stats.rxPackets > 0)
    {
      double duration =
          stats.timeLastRxPacket.GetSeconds() - stats.timeFirstTxPacket.GetSeconds();

      if (duration > 0)
      {
        totalThroughput += (stats.rxBytes * 8.0) / (duration * 1e3);
      }

      totalDelayMs += (stats.delaySum.GetSeconds() * 1000.0) / stats.rxPackets;
      totalJitterMs += (stats.jitterSum.GetSeconds() * 1000.0) / stats.rxPackets;
      flowCount++;
    }
  }

  double avgThroughputKbps = (flowCount > 0) ? (totalThroughput / flowCount) : 0.0;
  double avgDelayMs = (flowCount > 0) ? (totalDelayMs / flowCount) : 0.0;
  double avgJitterMs = (flowCount > 0) ? (totalJitterMs / flowCount) : 0.0;
  double pdr = (totalTxPackets > 0) ? static_cast<double>(totalRxPackets) / totalTxPackets : 0.0;

  uint64_t totalReq = 0;
  uint64_t totalResp = 0;
  uint64_t totalSucc = 0;
  uint64_t totalReject = 0;
  uint64_t totalTimeout = 0;
  double totalAuthE2E = 0.0;

  for (const auto& s : droneStats)
  {
    totalReq += s.requestsSent;
    totalResp += s.responsesReceived;
    totalSucc += s.authSuccess;
    totalReject += s.authRejected;
    totalTimeout += s.authTimeout;
    totalAuthE2E += s.totalAuthE2EDelayMs;
  }

  double avgAuthE2EDelayMs = (totalSucc > 0) ? (totalAuthE2E / totalSucc) : 0.0;
  double authSuccessRate = (totalReq > 0) ? static_cast<double>(totalSucc) / totalReq : 0.0;

  std::filesystem::path outPath(csvFile);
  if (outPath.has_parent_path())
  {
    std::filesystem::create_directories(outPath.parent_path());
  }

  bool writeHeader = !std::filesystem::exists(outPath);
  std::ofstream out(csvFile, std::ios::app);

  if (writeHeader)
  {
    out << "Run,Regions,DronesPerRegion,AVsPerRegion,SimTime,AppStopTime,"
        << "AuthIntervalMs,AuthTimeoutMs,FreshnessMs,ProcessingDelayMs,"
        << "ThroughputKbps,FlowDelayMs,FlowJitterMs,PDR,"
        << "AuthRequests,AuthResponses,AuthSuccess,AuthRejected,AuthTimeout,"
        << "AuthSuccessRate,AuthAvgE2EDelayMs\n";
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
      << processingDelayMs << ","
      << std::fixed << std::setprecision(6)
      << avgThroughputKbps << ","
      << avgDelayMs << ","
      << avgJitterMs << ","
      << pdr << ","
      << totalReq << ","
      << totalResp << ","
      << totalSucc << ","
      << totalReject << ","
      << totalTimeout << ","
      << authSuccessRate << ","
      << avgAuthE2EDelayMs << "\n";

  out.close();

  Simulator::Destroy();
  return 0;
}