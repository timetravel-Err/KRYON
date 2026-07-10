/**
 * UAV-Assisted IoAV Communication Simulation (2PQS-IoAV)
 * 
 * This simulation models realistic communication between UAVs (drones) and 
 * Autonomous Vehicles (AVs) for the 2PQS-IoAV authentication scheme.
 * 
 * Network Model:
 * - UAVs: Flying at 50-150m altitude, 5-15 m/s speed
 * - AVs: Mobile ground vehicles at road level (Z=0), 8-17 m/s (~30-60 km/h)
 * - WiFi: Ad-Hoc mode (IEEE 802.11n, 5 GHz) with MinstrelHt adaptive rate
 * - Propagation: Friis Free Space model for air-to-ground LOS
 * - TX Power: 23 dBm, RTS/CTS enabled for hidden-terminal mitigation
 * - Region: Scales with sqrt(nodes) to maintain realistic density
 *
 * Traffic Pattern (2PQS-IoAV Authentication & Key Agreement):
 * - Request (Drone → AV): mα = ⟨EIDα, VIDα, Tα, Cα⟩ = 6688 bits (836 bytes)
 * - Response (AV → Drone): mAV = ⟨EIDAV, SKVα−AV, TAV⟩ = 544 bits (68 bytes)
 * - Total AKA communication cost: 7232 bits (904 bytes)
 * - Auth interval: 100ms (10 auth exchanges/sec per AV)
 */
#include "../../KRYON/include/core/Version.h"
#include "../../KRYON/include/core/Logger.h"
#include "../../KRYON/include/core/ExperimentConfig.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/olsr-module.h"

#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("DroneAVSimulation");

int main(int argc, char *argv[])
{
    kryon::Logger::Info("==========================================");
	kryon::Logger::Info("Starting KRYON Research Framework");
	kryon::Logger::Info(kryon::Version::GetFrameworkVersion());
	kryon::Logger::Info(kryon::Version::GetNs3Version());
	kryon::Logger::Info("==========================================");
	

	kryon::ExperimentConfig config;
	
    config.Parse(argc, argv);

    // Enable WiFi logging for debugging (optional)
    // LogComponentEnable("StaWifiMac", LOG_LEVEL_INFO);

    RngSeedManager::SetSeed(1);
    RngSeedManager::SetRun(config.run);

    uint32_t totalDrones = config.numRegions * config.dronesPerRegion;
    uint32_t totalAVs = config.numRegions * config.avsPerRegion;

    // Create UAV (drone) nodes - act as flying Access Points
    NodeContainer drones;
    drones.Create(totalDrones);

    // Create Autonomous Vehicle nodes - act as mobile stations
    NodeContainer avs;
    avs.Create(totalAVs);

    // Install Internet stack with OLSR routing (required for ad-hoc network)
    OlsrHelper olsr;
    Ipv4StaticRoutingHelper staticRouting;
    Ipv4ListRoutingHelper routing;
    routing.Add(staticRouting, 0);
    routing.Add(olsr, 10);

    InternetStackHelper internet;
    internet.SetRoutingHelper(routing);
    internet.Install(drones);
    internet.Install(avs);

    /* ---------------- Mobility ---------------- */

    // UAV (Drone) Mobility: Random waypoint in 3D space (50-150m altitude)
    double droneAltitudeMin = 50.0;   // Minimum altitude in meters
    double droneAltitudeMax = 150.0;  // Maximum altitude in meters

    // Scale region radius with total node count to avoid unrealistic density
    // Base: 100m for ~20 nodes, grows with sqrt(nodes) to maintain ~1 node per 500m²
    uint32_t totalNodes = totalDrones + totalAVs;
    double regionRadius = std::max(100.0, 50.0 * std::sqrt((double)totalNodes));

    for (uint32_t r = 0; r < config.numRegions; ++r)
    {
        double regionCenterX = 500.0 * r;
        double regionCenterY = 0.0;

        for (uint32_t d = 0; d < config.dronesPerRegion; ++d)
        {
            uint32_t droneIdx = r * config.dronesPerRegion + d;

            MobilityHelper droneMobility;

            // 3D Position allocator for UAV waypoints (flying at altitude)
            Ptr<RandomBoxPositionAllocator> posAlloc =
                CreateObject<RandomBoxPositionAllocator>();
            posAlloc->SetAttribute(
                "X",
                StringValue("ns3::UniformRandomVariable[Min=" +
                            std::to_string(regionCenterX - regionRadius) + "|Max=" +
                            std::to_string(regionCenterX + regionRadius) + "]"));
            posAlloc->SetAttribute(
                "Y",
                StringValue("ns3::UniformRandomVariable[Min=" +
                            std::to_string(regionCenterY - regionRadius) + "|Max=" +
                            std::to_string(regionCenterY + regionRadius) + "]"));
            posAlloc->SetAttribute(
                "Z",
                StringValue("ns3::UniformRandomVariable[Min=" +
                            std::to_string(droneAltitudeMin) + "|Max=" +
                            std::to_string(droneAltitudeMax) + "]"));

            droneMobility.SetPositionAllocator(posAlloc);

            // UAV speed: 5-15 m/s (typical for small surveillance drones)
            droneMobility.SetMobilityModel(
                "ns3::RandomWaypointMobilityModel",
                "Speed",
                StringValue("ns3::UniformRandomVariable[Min=5|Max=15]"),
                "Pause",
                StringValue("ns3::ConstantRandomVariable[Constant=1.0]"),
                "PositionAllocator",
                PointerValue(posAlloc));

            droneMobility.Install(drones.Get(droneIdx));
        }

        // AV (Autonomous Vehicle) Mobility: Moving at ground level (Z=0)
        for (uint32_t a = 0; a < config.avsPerRegion; ++a)
        {
            uint32_t avIdx = r * config.avsPerRegion + a;

            MobilityHelper avMobility;

            // Ground-level position allocator for AVs (road surface, Z=0)
            Ptr<RandomBoxPositionAllocator> avPosAlloc =
                CreateObject<RandomBoxPositionAllocator>();
            avPosAlloc->SetAttribute(
                "X",
                StringValue("ns3::UniformRandomVariable[Min=" +
                            std::to_string(regionCenterX - regionRadius) + "|Max=" +
                            std::to_string(regionCenterX + regionRadius) + "]"));
            avPosAlloc->SetAttribute(
                "Y",
                StringValue("ns3::UniformRandomVariable[Min=" +
                            std::to_string(regionCenterY - regionRadius) + "|Max=" +
                            std::to_string(regionCenterY + regionRadius) + "]"));
            avPosAlloc->SetAttribute(
                "Z",
                StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));  // Ground level

            avMobility.SetPositionAllocator(avPosAlloc);

            // AV speed: 8-17 m/s (approximately 30-60 km/h, typical urban driving)
            avMobility.SetMobilityModel(
                "ns3::RandomWaypointMobilityModel",
                "Speed",
                StringValue("ns3::UniformRandomVariable[Min=8|Max=17]"),
                "Pause",
                StringValue("ns3::ConstantRandomVariable[Constant=0.5]"),
                "PositionAllocator",
                PointerValue(avPosAlloc));

            avMobility.Install(avs.Get(avIdx));
        }
    }

    /* ---------------- Network (WiFi Ad-Hoc) ---------------- */

    // WiFi configuration for UAV-AV air-to-ground communication
    // Using Ad-Hoc mode for realistic peer-to-peer mobile communication
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211n);
    // MinstrelHt: adaptive rate control — automatically selects best MCS
    // based on channel conditions (essential for mobile UAV-AV links)
    // RtsCtsThreshold=100: enables RTS/CTS for frames ≥100 bytes
    // (mitigates hidden-terminal problem in dense UAV-AV networks)
    wifi.SetRemoteStationManager("ns3::MinstrelHtWifiManager",
                                 "RtsCtsThreshold", UintegerValue(100));

    // Ad-Hoc MAC for all nodes (realistic for UAV-AV mesh network)
    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");

    // Realistic air-to-ground propagation model
    // Friis Free Space model: suitable for UAV LOS to ground (matches Table IV)
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    
    // Friis propagation loss model (line-of-sight, good for UAV-to-ground)
    // UAVs have clear LOS to ground vehicles in most cases
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel",
                                   "Frequency", DoubleValue(5.0e9));  // 5 GHz

    YansWifiPhyHelper wifiPhy;
    wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
    
    // TX power 23 dBm (~200 mW) for UAV communication range (50-150m altitude)
    wifiPhy.Set("TxPowerStart", DoubleValue(23.0));  // 23 dBm
    wifiPhy.Set("TxPowerEnd", DoubleValue(23.0));
    wifiPhy.Set("RxSensitivity", DoubleValue(-82.0)); // Good sensitivity

    // Single shared channel for the region (ad-hoc network)
    Ptr<YansWifiChannel> channel = wifiChannel.Create();
    wifiPhy.SetChannel(channel);

    Ipv4AddressHelper ipv4;
    
    // Install WiFi on all drones
    NetDeviceContainer droneDevices = wifi.Install(wifiPhy, wifiMac, drones);
    
    // Install WiFi on all AVs
    NetDeviceContainer avDevices = wifi.Install(wifiPhy, wifiMac, avs);

    // Assign IP addresses (single subnet for ad-hoc network)
    ipv4.SetBase("10.1.0.0", "255.255.0.0");  // Large subnet for all nodes
    
    Ipv4InterfaceContainer droneInterfaces = ipv4.Assign(droneDevices);
    Ipv4InterfaceContainer avInterfaces = ipv4.Assign(avDevices);

    /* ---------------- Applications (Bidirectional) ---------------- */

    // 2PQS-IoAV Authentication & Key Agreement (AKA) traffic:
    // - Drone sends mα = ⟨EIDα, VIDα, Tα, Cα⟩ = 6688 bits (836 bytes) to AV
    // - AV replies mAV = ⟨EIDAV, SKVα−AV, TAV⟩ = 544 bits (68 bytes) to Drone
    // - Total per exchange: 7232 bits (904 bytes)

    uint16_t requestPort = 9000;   // Drone → AV (AKA request: mα)
    uint16_t responsePort = 9001;  // AV → Drone (AKA response: mAV)

    // Install UDP servers on AVs (receive AKA requests from drones)
    for (uint32_t a = 0; a < totalAVs; ++a)
    {
        UdpServerHelper avServer(requestPort);
        ApplicationContainer serverApp = avServer.Install(avs.Get(a));
        serverApp.Start(Seconds(0.0));
        serverApp.Stop(Seconds(config.simTime));
    }

    // Install UDP servers on drones (receive AKA responses from AVs)
    for (uint32_t d = 0; d < totalDrones; ++d)
    {
        UdpServerHelper droneServer(responsePort);
        ApplicationContainer serverApp = droneServer.Install(drones.Get(d));
        serverApp.Start(Seconds(0.0));
        serverApp.Stop(Seconds(config.simTime));
    }

    // Each drone authenticates with its assigned AVs (round-robin distribution)
    // Start after OLSR routing converges (~10 seconds)
    for (uint32_t a = 0; a < totalAVs; ++a)
    {
        // Determine which drone this AV communicates with
        uint32_t targetDrone = a % totalDrones;  // Distribute AVs across drones
        
        // AKA Step 1: Drone → AV  mα = ⟨EIDα, VIDα, Tα, Cα⟩ = 6688 bits = 836 bytes
        // Auth exchange every 100ms (10 authentications/sec per AV)
        UdpClientHelper droneClient(avInterfaces.GetAddress(a), requestPort);
        droneClient.SetAttribute("MaxPackets", UintegerValue(0));
        droneClient.SetAttribute("Interval", TimeValue(MilliSeconds(100)));
        droneClient.SetAttribute("PacketSize", UintegerValue(836));

        ApplicationContainer droneApp = droneClient.Install(drones.Get(targetDrone));
        droneApp.Start(Seconds(10.0 + 0.1 * a));  // Start after OLSR convergence
        droneApp.Stop(Seconds(config.simTime));

        // AKA Step 2: AV → Drone  mAV = ⟨EIDAV, SKVα−AV, TAV⟩ = 544 bits = 68 bytes
        UdpClientHelper avClient(droneInterfaces.GetAddress(targetDrone), responsePort);
        avClient.SetAttribute("MaxPackets", UintegerValue(0));
        avClient.SetAttribute("Interval", TimeValue(MilliSeconds(100)));  // One response per request
        avClient.SetAttribute("PacketSize", UintegerValue(68));

        ApplicationContainer avApp = avClient.Install(avs.Get(a));
        avApp.Start(Seconds(10.5 + 0.1 * a));  // Start after drone sends first
        avApp.Stop(Seconds(config.simTime));
    }

    /* ---------------- Flow Monitor ---------------- */

    FlowMonitorHelper flowHelper;
    Ptr<FlowMonitor> monitor = flowHelper.InstallAll();

    Simulator::Stop(Seconds(config.simTime));
    Simulator::Run();

    monitor->CheckForLostPackets();

    double totalThroughput = 0.0;
    double totalDelay = 0.0;
    double totalJitter = 0.0;
    uint64_t totalTx = 0;
    uint64_t totalRx = 0;
    uint32_t flowCount = 0;

    for (auto const &flow : monitor->GetFlowStats())
    {
        const FlowMonitor::FlowStats &stats = flow.second;

        totalTx += stats.txPackets;
        totalRx += stats.rxPackets;

        if (stats.rxPackets > 0)
        {
            double duration =
                stats.timeLastRxPacket.GetSeconds() -
                stats.timeFirstTxPacket.GetSeconds();

            if (duration > 0)
            {
                // Throughput in Kbps
                totalThroughput += (stats.rxBytes * 8.0) / (duration * 1e3);
            }
            // Delay in milliseconds (delaySum is total delay for all packets)
            totalDelay += (stats.delaySum.GetSeconds() * 1000.0) / stats.rxPackets;
            // Jitter in milliseconds (jitterSum is total jitter for all packets)
            totalJitter += (stats.jitterSum.GetSeconds() * 1000.0) / stats.rxPackets;
            flowCount++;
        }
    }

    double avgThroughput = (flowCount > 0) ? totalThroughput / flowCount : 0.0;
    double avgDelay = (flowCount > 0) ? totalDelay / flowCount : 0.0;
    double avgJitter = (flowCount > 0) ? totalJitter / flowCount : 0.0;
    double pdr = (totalTx > 0) ? double(totalRx) / totalTx : 0.0;

    bool writeHeader = !std::ifstream(config.csvFile).good();
    std::ofstream out(config.csvFile, std::ios::app);

    if (writeHeader)
    {
        out << "Regions,Drones,AVs,Throughput,Delay,Jitter,PDR\n";
    }

    out << config.numRegions << ","
        << config.dronesPerRegion << ","
        << config.avsPerRegion << ","
        << avgThroughput << ","
        << avgDelay << ","
        << avgJitter << ","
        << pdr << "\n";

    out.close();

    Simulator::Destroy();
    return 0;
}
