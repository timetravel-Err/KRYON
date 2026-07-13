#ifndef KRYON_REGION_MANAGER_H
#define KRYON_REGION_MANAGER_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : RegionManager.h
 *
 * Description:
 * Creates the simulation topology and installs the
 * Internet stack.
 *
 * Responsibilities
 * ----------------
 * • Calculate topology size
 * • Create UAV nodes
 * • Create AV nodes
 * • Install Internet stack
 * • Configure OLSR routing
 *
 * Version : 0.1.0
 * ----------------------------------------------------------
 */

#include "../core/ExperimentConfig.h"
#include "../simulation/SimulationContext.h"

#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/olsr-module.h"

namespace kryon
{

class RegionManager
{
public:

    RegionManager(const ExperimentConfig& config,
                  SimulationContext& context)
        : m_config(config),
          m_context(context)
    {
    }

    void Initialize()
    {
        CalculateTopology();

        CreateNodes();

        InstallInternetStack();
    }

private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;

    void CalculateTopology()
    {
        m_context.totalDrones =
            m_config.numRegions *
            m_config.dronesPerRegion;

        m_context.totalAVs =
            m_config.numRegions *
            m_config.avsPerRegion;
    }

    void CreateNodes()
    {
        m_context.drones.Create(m_context.totalDrones);

        m_context.avs.Create(m_context.totalAVs);
    }

    void InstallInternetStack()
    {
        ns3::OlsrHelper olsr;

        ns3::Ipv4StaticRoutingHelper staticRouting;

        ns3::Ipv4ListRoutingHelper routing;

        routing.Add(staticRouting, 0);

        routing.Add(olsr, 10);

        ns3::InternetStackHelper internet;

        internet.SetRoutingHelper(routing);

        internet.Install(m_context.drones);

        internet.Install(m_context.avs);
    }
};

}

#endif
