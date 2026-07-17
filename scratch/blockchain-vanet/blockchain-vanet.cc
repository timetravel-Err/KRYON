// blockchain-vanet.cc
//
// NS-3 simulation of a blockchain-secured authentication framework for
// RSU <-> vehicle / RSU <-> drone / RSU <-> RSU communication in a VANET.
//
//  - RSUs run PBFT-lite consensus over a shared ledger (blockchain.h).
//  - Vehicles/drones authenticate via CA-issued ECDSA certificates
//    (mobile-node-app.h) and, once authenticated, send HMAC-tagged
//    telemetry using a session token derived from the committed block.
//  - Optional Sybil / Replay / DoS attacker nodes stress-test the RSU's
//    defenses (attack-apps.h).
//  - Metrics: application-layer (metrics.h -> CSV) + network-layer
//    (ns-3 FlowMonitor -> XML), both written to --outDir.
//
// Usage (from ns-3 root, after copying this folder into scratch/):
//   ./ns3 run "blockchain-vanet --numVehicles=20 --numDrones=5 --numRsu=4
//              --simTime=60 --enableSybil=true --enableReplay=true --enableDos=true"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/csma-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/pointer.h"
#include "ns3/double.h"

#include "crypto-utils.h"
#include "blockchain.h"
#include "metrics.h"
#include "rsu-app.h"
#include "mobile-node-app.h"
#include "attack-apps.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("BlockchainVanetMain");

int
main (int argc, char *argv[])
{
  uint32_t numVehicles = 20;
  uint32_t numDrones = 5;
  uint32_t numRsu = 4;
  double simTime = 60.0;
  bool enableSybil = true;
  bool enableReplay = true;
  bool enableDos = true;
  std::string outDir = "./results";
  uint16_t rsuPort = 9000;

  CommandLine cmd;
  cmd.AddValue ("numVehicles", "Number of vehicle nodes", numVehicles);
  cmd.AddValue ("numDrones", "Number of drone nodes", numDrones);
  cmd.AddValue ("numRsu", "Number of RSU (PBFT validator) nodes", numRsu);
  cmd.AddValue ("simTime", "Simulation duration in seconds", simTime);
  cmd.AddValue ("enableSybil", "Enable Sybil attacker node", enableSybil);
  cmd.AddValue ("enableReplay", "Enable Replay attacker node", enableReplay);
  cmd.AddValue ("enableDos", "Enable DoS flood attacker node", enableDos);
  cmd.AddValue ("outDir", "Directory to write metrics CSV / FlowMonitor XML", outDir);
  cmd.Parse (argc, argv);

  int mkdirRc = system (("mkdir -p " + outDir).c_str ());
  (void) mkdirRc;
  MetricsCollector::Get ().SetOutputDir (outDir);

  // -------------------------------------------------------------------
  // 1. PKI bootstrap: simulated root CA + certificates for every RSU/
  //    vehicle/drone. In a real deployment this happens out-of-band
  //    (e.g. at vehicle registration/inspection); here we do it once at
  //    t=0 in main() before the simulation clock starts.
  // -------------------------------------------------------------------
  CryptoUtils::KeyPair caKp = CryptoUtils::GenerateKeyPair ();
  std::vector<uint8_t> caPubBytes = CryptoUtils::ExportPublicKey (caKp);

  auto issueCert = [&] (uint32_t nodeId, const std::string &nodeType,
                         const CryptoUtils::KeyPair &kp) {
    Certificate c;
    c.nodeId = nodeId; c.nodeType = nodeType;
    c.publicKey = CryptoUtils::ExportPublicKey (kp);
    c.issuedAt = 0.0; c.expiresAt = simTime + 3600.0;
    c.caSignature = CryptoUtils::Sign (caKp, c.SerializeForSigning ());
    return c;
  };

  // -------------------------------------------------------------------
  // 2. Node creation & mobility
  // -------------------------------------------------------------------
  NodeContainer rsuNodes;      rsuNodes.Create (numRsu);
  NodeContainer vehicleNodes;  vehicleNodes.Create (numVehicles);
  NodeContainer droneNodes;    droneNodes.Create (numDrones);
  NodeContainer attackerNodes; attackerNodes.Create (3); // sybil, replay, dos (unused ones idle)

  NodeContainer allNodes;
  allNodes.Add (rsuNodes); allNodes.Add (vehicleNodes);
  allNodes.Add (droneNodes); allNodes.Add (attackerNodes);

  // RSUs: fixed positions spaced along a road (x-axis), y=0.
  MobilityHelper rsuMobility;
  Ptr<ListPositionAllocator> rsuPos = CreateObject<ListPositionAllocator> ();
  for (uint32_t i = 0; i < numRsu; ++i)
    rsuPos->Add (Vector (i * 300.0, 0.0, 10.0)); // RSUs mounted at height 10m
  rsuMobility.SetPositionAllocator (rsuPos);
  rsuMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  rsuMobility.Install (rsuNodes);

  double roadLength = (numRsu - 1) * 300.0 + 100.0;

  // Vehicles: move along the road (highway-like) using RandomWaypoint
  // within the RSU corridor. Built programmatically (not via nested
  // attribute strings) to avoid ns-3's string-based ObjectFactory parser
  // choking on multi-level nested "ns3::X[attr=ns3::Y[...]]" specs.
  Ptr<UniformRandomVariable> vehXRv = CreateObject<UniformRandomVariable> ();
  vehXRv->SetAttribute ("Min", DoubleValue (0.0));
  vehXRv->SetAttribute ("Max", DoubleValue (roadLength));
  Ptr<UniformRandomVariable> vehYRv = CreateObject<UniformRandomVariable> ();
  vehYRv->SetAttribute ("Min", DoubleValue (-20.0));
  vehYRv->SetAttribute ("Max", DoubleValue (20.0));

  Ptr<RandomRectanglePositionAllocator> vehPosAlloc = CreateObject<RandomRectanglePositionAllocator> ();
  vehPosAlloc->SetAttribute ("X", PointerValue (vehXRv));
  vehPosAlloc->SetAttribute ("Y", PointerValue (vehYRv));

  Ptr<UniformRandomVariable> vehSpeedRv = CreateObject<UniformRandomVariable> ();
  vehSpeedRv->SetAttribute ("Min", DoubleValue (10.0));
  vehSpeedRv->SetAttribute ("Max", DoubleValue (25.0));
  Ptr<ConstantRandomVariable> vehPauseRv = CreateObject<ConstantRandomVariable> ();
  vehPauseRv->SetAttribute ("Constant", DoubleValue (0.5));

  MobilityHelper vehMobility;
  vehMobility.SetPositionAllocator (vehPosAlloc);
  vehMobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
    "Speed", PointerValue (vehSpeedRv),
    "Pause", PointerValue (vehPauseRv),
    "PositionAllocator", PointerValue (vehPosAlloc));
  vehMobility.Install (vehicleNodes);

  // Drones: 3D random waypoint at altitude (same programmatic approach).
  Ptr<UniformRandomVariable> droneXRv = CreateObject<UniformRandomVariable> ();
  droneXRv->SetAttribute ("Min", DoubleValue (0.0));
  droneXRv->SetAttribute ("Max", DoubleValue (roadLength));
  Ptr<UniformRandomVariable> droneYRv = CreateObject<UniformRandomVariable> ();
  droneYRv->SetAttribute ("Min", DoubleValue (-50.0));
  droneYRv->SetAttribute ("Max", DoubleValue (50.0));
  Ptr<UniformRandomVariable> droneZRv = CreateObject<UniformRandomVariable> ();
  droneZRv->SetAttribute ("Min", DoubleValue (30.0));
  droneZRv->SetAttribute ("Max", DoubleValue (80.0));

  Ptr<RandomBoxPositionAllocator> dronePosAlloc = CreateObject<RandomBoxPositionAllocator> ();
  dronePosAlloc->SetAttribute ("X", PointerValue (droneXRv));
  dronePosAlloc->SetAttribute ("Y", PointerValue (droneYRv));
  dronePosAlloc->SetAttribute ("Z", PointerValue (droneZRv));

  Ptr<UniformRandomVariable> droneSpeedRv = CreateObject<UniformRandomVariable> ();
  droneSpeedRv->SetAttribute ("Min", DoubleValue (5.0));
  droneSpeedRv->SetAttribute ("Max", DoubleValue (15.0));
  Ptr<ConstantRandomVariable> dronePauseRv = CreateObject<ConstantRandomVariable> ();
  dronePauseRv->SetAttribute ("Constant", DoubleValue (1.0));

  MobilityHelper droneMobility;
  droneMobility.SetPositionAllocator (dronePosAlloc);
  droneMobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
    "Speed", PointerValue (droneSpeedRv),
    "Pause", PointerValue (dronePauseRv),
    "PositionAllocator", PointerValue (dronePosAlloc));
  droneMobility.Install (droneNodes);

  MobilityHelper attackerMobility;
  attackerMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  attackerMobility.Install (attackerNodes);
  for (uint32_t i = 0; i < attackerNodes.GetN (); ++i)
    attackerNodes.Get (i)->GetObject<ConstantPositionMobilityModel> ()->SetPosition (Vector (150.0, 5.0 * i, 0.0));

  // -------------------------------------------------------------------
  // 3. Wireless network (802.11 ad-hoc standing in for 802.11p/WAVE; swap
  //    WifiStandard below to WIFI_STANDARD_80211p if your ns-3 build has
  //    the 'wave' module compiled in).
  // -------------------------------------------------------------------
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy;
  phy.SetChannel (channel.Create ());

  WifiHelper wifi;
  wifi.SetStandard (WIFI_STANDARD_80211g);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                 "DataMode", StringValue ("ErpOfdmRate24Mbps"));

  WifiMacHelper mac;
  mac.SetType ("ns3::AdhocWifiMac");

  NetDeviceContainer devices = wifi.Install (phy, mac, allNodes);

  // Dedicated wired backbone for RSU <-> RSU PBFT consensus traffic. This
  // mirrors a real deployment (RSUs are normally linked by a wired
  // backhaul) and, importantly, avoids relying on WiFi radio range for
  // consensus gossip - with RSUs spaced hundreds of meters apart on a
  // shared wireless channel, non-adjacent RSUs may simply be unable to
  // hear each other at the PHY layer, which would silently starve PBFT of
  // the votes it needs to ever reach quorum.
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (200)));
  NetDeviceContainer rsuBackboneDevices = csma.Install (rsuNodes);

  InternetStackHelper internet;
  internet.Install (allNodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

  Ipv4AddressHelper ipv4Backbone;
  ipv4Backbone.SetBase ("10.2.1.0", "255.255.255.0");
  Ipv4InterfaceContainer backboneInterfaces = ipv4Backbone.Assign (rsuBackboneDevices);

  // -------------------------------------------------------------------
  // 4. RSU setup: keypairs, certs, peer list for PBFT gossip.
  //    - rsuAddrs (WiFi IP)      -> what vehicles/drones send AUTH_REQUEST/
  //                                 DATA_MSG to.
  //    - allRsuPeerInfo (backbone IP) -> what RSUs use to gossip PBFT
  //                                 PRE_PREPARE/PREPARE/COMMIT/BLOCK_SYNC
  //                                 with each other.
  //    A single UDP socket bound to 0.0.0.0:port on each RSU receives
  //    traffic from both interfaces, so no extra socket is needed.
  // -------------------------------------------------------------------
  std::vector<CryptoUtils::KeyPair> rsuKeys (numRsu);
  std::vector<Address> rsuAddrs (numRsu);
  std::vector<RsuApp::PeerRsu> allRsuPeerInfo (numRsu);

  for (uint32_t i = 0; i < numRsu; ++i)
    {
      rsuKeys[i] = CryptoUtils::GenerateKeyPair ();
      Ipv4Address wifiIp = interfaces.GetAddress (i); // RSU nodes are allNodes indices [0, numRsu)
      Ipv4Address backboneIp = backboneInterfaces.GetAddress (i);
      rsuAddrs[i] = InetSocketAddress (wifiIp, rsuPort);
      Address backboneAddr = InetSocketAddress (backboneIp, rsuPort);
      allRsuPeerInfo[i] = {i, backboneAddr, CryptoUtils::ExportPublicKey (rsuKeys[i])};
    }

  ApplicationContainer rsuApps;
  for (uint32_t i = 0; i < numRsu; ++i)
    {
      Ptr<RsuApp> app = CreateObject<RsuApp> ();
      std::vector<RsuApp::PeerRsu> peers;
      for (uint32_t j = 0; j < numRsu; ++j) if (j != i) peers.push_back (allRsuPeerInfo[j]);
      app->Setup (i, rsuKeys[i], caPubBytes, peers, rsuPort, numRsu);
      rsuNodes.Get (i)->AddApplication (app);
      app->SetStartTime (Seconds (0.0));
      app->SetStopTime (Seconds (simTime));
      rsuApps.Add (app);
    }

  // -------------------------------------------------------------------
  // 5. Vehicles / drones: keypair + CA cert + home-RSU assignment
  //    (assigned round-robin here; swap for nearest-RSU-by-position for a
  //    more realistic handover-aware v2 model).
  // -------------------------------------------------------------------
  std::vector<uint8_t> capturedReplayPacket; // filled in for node 0 (replay victim)

  for (uint32_t i = 0; i < numVehicles; ++i)
    {
      uint32_t nodeId = 1000 + i;
      CryptoUtils::KeyPair kp = CryptoUtils::GenerateKeyPair ();
      Certificate cert = issueCert (nodeId, "vehicle", kp);
      uint32_t homeRsu = i % numRsu;

      Ptr<MobileNodeApp> app = CreateObject<MobileNodeApp> ();
      app->Setup (nodeId, "vehicle", kp, cert, rsuAddrs[homeRsu], Seconds (1.0));
      vehicleNodes.Get (i)->AddApplication (app);
      app->SetStartTime (Seconds (0.0));
      app->SetStopTime (Seconds (simTime));

      if (i == 0 && enableReplay)
        {
          // Simulate an eavesdropper capturing THIS vehicle's very first
          // AUTH_REQUEST on the wire: build the identical, validly-signed
          // packet bytes here (same identity/keys/nonce=1) to hand to the
          // ReplayAttackApp. The attacker never sees the private key -
          // only the resulting wire bytes, exactly like a sniffed packet.
          AuthRequestMsg captured;
          captured.nodeId = nodeId; captured.nodeType = "vehicle";
          captured.timestamp = 0.5; captured.nonce = 1;
          ByteWriter w; w.U32 (captured.nodeId); w.Str (captured.nodeType);
          w.Dbl (captured.timestamp); w.U64 (captured.nonce);
          captured.signature = CryptoUtils::Sign (kp, w.buf);
          captured.cert = cert;
          capturedReplayPacket = captured.Serialize ();
        }
    }

  for (uint32_t i = 0; i < numDrones; ++i)
    {
      uint32_t nodeId = 2000 + i;
      CryptoUtils::KeyPair kp = CryptoUtils::GenerateKeyPair ();
      Certificate cert = issueCert (nodeId, "drone", kp);
      uint32_t homeRsu = i % numRsu;

      Ptr<MobileNodeApp> app = CreateObject<MobileNodeApp> ();
      app->Setup (nodeId, "drone", kp, cert, rsuAddrs[homeRsu], Seconds (1.5));
      droneNodes.Get (i)->AddApplication (app);
      app->SetStartTime (Seconds (0.0));
      app->SetStopTime (Seconds (simTime));
    }

  // -------------------------------------------------------------------
  // 6. Attackers (target RSU 0 by default)
  // -------------------------------------------------------------------
  if (enableSybil)
    {
      Ptr<SybilAttackApp> app = CreateObject<SybilAttackApp> ();
      app->Setup (rsuAddrs[0], 30, MilliSeconds (200));
      attackerNodes.Get (0)->AddApplication (app);
      app->SetStartTime (Seconds (5.0));
      app->SetStopTime (Seconds (std::min (simTime, 25.0)));
    }

  if (enableReplay && !capturedReplayPacket.empty ())
    {
      Ptr<ReplayAttackApp> app = CreateObject<ReplayAttackApp> ();
      app->Setup (rsuAddrs[0], capturedReplayPacket,
                  {Seconds (10.0), Seconds (15.0), Seconds (20.0)});
      attackerNodes.Get (1)->AddApplication (app);
      app->SetStartTime (Seconds (1.0));
      app->SetStopTime (Seconds (simTime));
    }

  if (enableDos)
    {
      Ptr<DosFloodAttackApp> app = CreateObject<DosFloodAttackApp> ();
      app->Setup (rsuAddrs[0], MilliSeconds (5), 256, Seconds (10.0));
      attackerNodes.Get (2)->AddApplication (app);
      app->SetStartTime (Seconds (30.0));
      app->SetStopTime (Seconds (std::min (simTime, 40.0)));
    }

  // -------------------------------------------------------------------
  // 7. FlowMonitor (network-layer PDR / throughput / delay)
  // -------------------------------------------------------------------
  FlowMonitorHelper flowmonHelper;
  Ptr<FlowMonitor> flowmon = flowmonHelper.InstallAll ();

  Simulator::Stop (Seconds (simTime + 2.0));
  Simulator::Run ();

  flowmon->CheckForLostPackets ();
  flowmon->SerializeToXmlFile (outDir + "/flowmon.xml", true, true);
  MetricsCollector::Get ().Flush ();

  Simulator::Destroy ();

  CryptoUtils::FreeKeyPair (caKp);
  for (auto &kp : rsuKeys) CryptoUtils::FreeKeyPair (const_cast<CryptoUtils::KeyPair &> (kp));

  std::cout << "Simulation complete. Metrics in: " << outDir << std::endl;
  return 0;
}
