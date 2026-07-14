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
#include "../../KRYON/include/core/Version.h"
#include "../../KRYON/include/core/Logger.h"
#include "../../KRYON/include/core/ExperimentConfig.h"
#include "../../KRYON/include/simulation/SimulationContext.h"
#include "../../KRYON/include/region/RegionManager.h"
#include "../../KRYON/include/mobility/MobilityEngine.h"
#include "../../KRYON/include/network/CommunicationEngine.h"
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
	
	kryon::SimulationContext context;

    // Enable WiFi logging for debugging (optional)
    // LogComponentEnable("StaWifiMac", LOG_LEVEL_INFO);

    RngSeedManager::SetSeed(1);
    RngSeedManager::SetRun(config.run);

	kryon::RegionManager region(config, context);

	region.Initialize();
	auto& drones = context.drones;
	auto& avs = context.avs;

	uint32_t totalDrones = context.totalDrones;
	uint32_t totalAVs = context.totalAVs;

    /* ---------------- Mobility ---------------- */

	kryon::MobilityEngine mobility(config, context);

	mobility.Initialize();
	
	/* ---------------- Network (WiFi Ad-Hoc) ---------------- */
	
	kryon::CommunicationEngine communication(config, context);

	communication.Initialize();
    
    //auto& droneDevices = context.droneDevices;

	//auto& avDevices = context.avDevices;
	
    auto& droneInterfaces = context.droneInterfaces;

	auto& avInterfaces = context.avInterfaces;
	
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
