#ifndef KRYON_METRICS_ENGINE_H
#define KRYON_METRICS_ENGINE_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : MetricsEngine.h
 *
 * Description
 * -----------
 * Responsible for collecting simulation metrics.
 *
 * Current Support
 * ---------------
 * • FlowMonitor (future)
 * • Throughput
 * • Delay
 * • Jitter
 * • Packet Delivery Ratio
 *
 * Future Support
 * --------------
 * • Packet Loss Ratio
 * • Routing Overhead
 * • Authentication Latency
 * • Security Overhead
 * • Energy Consumption
 * • CSV Export
 *
 * Version : 0.1.0
 * ----------------------------------------------------------
 */

#include "../core/ExperimentConfig.h"
#include "../simulation/SimulationContext.h"
#include "../core/Logger.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/core-module.h"
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace kryon
{

class MetricsEngine
{
public:

    MetricsEngine(const ExperimentConfig& config,
                  SimulationContext& context)
        : m_config(config),
          m_context(context)
    {
    }

    void Initialize()
{
    InstallFlowMonitor();

    Logger::Info("Metrics Engine initialized.");
}

void RunSimulation()
{
    ns3::Simulator::Stop(
        ns3::Seconds(m_config.simTime));

    ns3::Simulator::Run();
}

void Finalize()
{
    m_context.monitor->CheckForLostPackets();

    Logger::Info("Flow statistics collected.");
}

void ComputeMetrics()
{
    double totalThroughput = 0.0;
    double totalDelay = 0.0;
    double totalJitter = 0.0;

    uint64_t totalTx = 0;
    uint64_t totalRx = 0;

    uint32_t flowCount = 0;

    for (auto const& flow : m_context.monitor->GetFlowStats())
    {
        const ns3::FlowMonitor::FlowStats& stats = flow.second;

        totalTx += stats.txPackets;
        totalRx += stats.rxPackets;

        if (stats.rxPackets > 0)
        {
            double duration =
                stats.timeLastRxPacket.GetSeconds() -
                stats.timeFirstTxPacket.GetSeconds();

            if (duration > 0)
            {
                totalThroughput +=
                    (stats.rxBytes * 8.0) /
                    (duration * 1000.0);
            }

            totalDelay +=
                (stats.delaySum.GetSeconds() * 1000.0) /
                stats.rxPackets;

            totalJitter +=
                (stats.jitterSum.GetSeconds() * 1000.0) /
                stats.rxPackets;

            flowCount++;
        }
    }

    m_context.metrics.throughput =
        (flowCount > 0) ?
        totalThroughput / flowCount : 0.0;

    m_context.metrics.delay =
        (flowCount > 0) ?
        totalDelay / flowCount : 0.0;

    m_context.metrics.jitter =
        (flowCount > 0) ?
        totalJitter / flowCount : 0.0;

    m_context.metrics.pdr =
        (totalTx > 0) ?
        double(totalRx) / totalTx : 0.0;

    Logger::Info("Performance metrics computed.");
}

void ExportResults()
{
    bool writeHeader =
        !std::ifstream(m_config.csvFile).good();

    std::ofstream out(
        m_config.csvFile,
        std::ios::app);
		
	/* ---------- Timestamp ---------- */

auto now =
    std::chrono::system_clock::now();

std::time_t nowTime =
    std::chrono::system_clock::to_time_t(now);

std::ostringstream timestamp;

timestamp << std::put_time(
    std::localtime(&nowTime),
    "%Y-%m-%d %H:%M:%S");	

   if (writeHeader)
{
    out << "FrameworkVersion,"
    << "Timestamp,"
    << "Run,"
    << "SimulationTime,"
    << "Regions,"
    << "Drones,"
    << "AVs,"
    << "Protocol,"
    << "Throughput,"
    << "Delay,"
    << "Jitter,"
    << "PDR,"
    << "AuthTimeMs,"
    << "AuthMessages,"
    << "AuthBytes,"
    << "AuthSuccess\n";
}
out << "1.1.0" << ","
    << timestamp.str() << ","
    << m_config.run << ","
    << m_config.simTime << ","
    << m_config.numRegions << ","
    << m_config.dronesPerRegion << ","
    << m_config.avsPerRegion << ","
    << m_context.metrics.authenticationProtocol << ","
    << m_context.metrics.throughput << ","
    << m_context.metrics.delay << ","
    << m_context.metrics.jitter << ","
    << m_context.metrics.pdr << ","
    << m_context.metrics.authenticationTimeMs << ","
    << m_context.metrics.authenticationMessages << ","
    << m_context.metrics.authenticationBytes << ","
    << (m_context.metrics.authenticationSuccess ? 1 : 0)
    << "\n";

    out.close();

    Logger::Info("Results exported.");
}

void DestroySimulation()
{
    ns3::Simulator::Destroy();

    Logger::Info("Simulation destroyed.");
}

private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;
	
	ns3::FlowMonitorHelper helper;
	
	void InstallFlowMonitor()
{
   
    m_context.monitor =
        helper.InstallAll();

    Logger::Info("FlowMonitor installed.");
}



};

}

#endif
