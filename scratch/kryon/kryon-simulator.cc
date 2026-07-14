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
#include "../../KRYON/include/protocol/ApplicationEngine.h"
#include "../../KRYON/include/metrics/MetricsEngine.h"
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
	
    /* ---------------- Mobility ---------------- */

	kryon::MobilityEngine mobility(config, context);

	mobility.Initialize();
	
	/* ---------------- Network (WiFi Ad-Hoc) ---------------- */
	
	kryon::CommunicationEngine communication(config, context);

	communication.Initialize();
    
	
    /* ---------------- Applications (Bidirectional) ---------------- */
	
	kryon::ApplicationEngine application(config, context);

	application.Initialize();

    // 2PQS-IoAV Authentication & Key Agreement (AKA) traffic:
    // - Drone sends mα = ⟨EIDα, VIDα, Tα, Cα⟩ = 6688 bits (836 bytes) to AV
    // - AV replies mAV = ⟨EIDAV, SKVα−AV, TAV⟩ = 544 bits (68 bytes) to Drone
    // - Total per exchange: 7232 bits (904 bytes)

   /* ---------------- Metrics ---------------- */

	kryon::MetricsEngine metrics(config, context);

	metrics.Initialize();

    /* ---------------- Flow Monitor ---------------- */

    auto monitor = context.monitor;
	
    metrics.RunSimulation();

    metrics.Finalize();

    metrics.ComputeMetrics();
	
    metrics.ExportResults();

    metrics.DestroySimulation();
	
    return 0;
}
