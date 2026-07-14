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
