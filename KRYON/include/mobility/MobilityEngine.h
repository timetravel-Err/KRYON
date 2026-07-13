//
#ifndef KRYON_MOBILITY_ENGINE_H
#define KRYON_MOBILITY_ENGINE_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : MobilityEngine.h
 *
 * Description
 * -----------
 * Responsible for installing all mobility models used in
 * the simulation.
 *
 * Current Support
 * ---------------
 * • UAV Random Waypoint
 * • AV Random Waypoint
 *
 * Future Support
 * --------------
 * • SUMO
 * • Gauss-Markov
 * • Manhattan Grid
 * • Drone Swarm
 *
 * Version : 0.1.0
 * ----------------------------------------------------------
 */

#include "../core/ExperimentConfig.h"
#include "../simulation/SimulationContext.h"

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "../core/Logger.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace kryon
{

class MobilityEngine
{
public:

    MobilityEngine(const ExperimentConfig& config,
                   SimulationContext& context)
        : m_config(config),
          m_context(context)
    {
    }

    void Initialize()
    {
        CalculateRegionRadius();

        InstallDroneMobility();

        InstallVehicleMobility();
    }

	private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;

    //--------------------------------------------------------

  void CalculateRegionRadius()
{
    uint32_t totalNodes =
        m_context.totalDrones +
        m_context.totalAVs;

    // Scale region radius with node density.
    // Base radius = 100 m.
    // Increase with sqrt(total nodes) to avoid
    // unrealistically dense deployments.

    m_context.regionRadius =
        std::max(
            100.0,
            50.0 * std::sqrt(static_cast<double>(totalNodes))
        );

   Logger::Info(
    "Region Radius = " +
    std::to_string(m_context.regionRadius) +
    " meters");
}
    //--------------------------------------------------------
void InstallDroneMobility()
{
    for (uint32_t r = 0; r < m_config.numRegions; ++r)
    {
        double regionCenterX = 500.0 * r;
        double regionCenterY = 0.0;

        for (uint32_t d = 0; d < m_config.dronesPerRegion; ++d)
        {
            uint32_t droneIdx =
                r * m_config.dronesPerRegion + d;

            ns3::MobilityHelper droneMobility;

            ns3::Ptr<ns3::RandomBoxPositionAllocator> posAlloc =
                ns3::CreateObject<ns3::RandomBoxPositionAllocator>();

            posAlloc->SetAttribute(
                "X",
                ns3::StringValue(
                    "ns3::UniformRandomVariable[Min=" +
                    std::to_string(regionCenterX - m_context.regionRadius) +
                    "|Max=" +
                    std::to_string(regionCenterX + m_context.regionRadius) +
                    "]"));

            posAlloc->SetAttribute(
                "Y",
                ns3::StringValue(
                    "ns3::UniformRandomVariable[Min=" +
                    std::to_string(regionCenterY - m_context.regionRadius) +
                    "|Max=" +
                    std::to_string(regionCenterY + m_context.regionRadius) +
                    "]"));

            posAlloc->SetAttribute(
                "Z",
                ns3::StringValue(
                    "ns3::UniformRandomVariable[Min=" +
                    std::to_string(m_context.droneAltitudeMin) +
                    "|Max=" +
                    std::to_string(m_context.droneAltitudeMax) +
                    "]"));

            droneMobility.SetPositionAllocator(posAlloc);

            droneMobility.SetMobilityModel(
                "ns3::RandomWaypointMobilityModel",
                "Speed",
                ns3::StringValue(
                    "ns3::UniformRandomVariable[Min=5|Max=15]"),
                "Pause",
                ns3::StringValue(
                    "ns3::ConstantRandomVariable[Constant=1.0]"),
                "PositionAllocator",
                ns3::PointerValue(posAlloc));

            droneMobility.Install(
                m_context.drones.Get(droneIdx));
        }
    }

    Logger::Info("Drone mobility installed.");
}

    //--------------------------------------------------------

   void InstallVehicleMobility()
{
    for (uint32_t r = 0; r < m_config.numRegions; ++r)
    {
        double regionCenterX = 500.0 * r;
        double regionCenterY = 0.0;

        for (uint32_t a = 0; a < m_config.avsPerRegion; ++a)
        {
            uint32_t avIdx =
                r * m_config.avsPerRegion + a;

            ns3::MobilityHelper avMobility;

            ns3::Ptr<ns3::RandomBoxPositionAllocator> avPosAlloc =
                ns3::CreateObject<ns3::RandomBoxPositionAllocator>();

            avPosAlloc->SetAttribute(
                "X",
                ns3::StringValue(
                    "ns3::UniformRandomVariable[Min=" +
                    std::to_string(regionCenterX - m_context.regionRadius) +
                    "|Max=" +
                    std::to_string(regionCenterX + m_context.regionRadius) +
                    "]"));

            avPosAlloc->SetAttribute(
                "Y",
                ns3::StringValue(
                    "ns3::UniformRandomVariable[Min=" +
                    std::to_string(regionCenterY - m_context.regionRadius) +
                    "|Max=" +
                    std::to_string(regionCenterY + m_context.regionRadius) +
                    "]"));

            avPosAlloc->SetAttribute(
                "Z",
                ns3::StringValue(
                    "ns3::ConstantRandomVariable[Constant=0.0]"));

            avMobility.SetPositionAllocator(avPosAlloc);

            avMobility.SetMobilityModel(
                "ns3::RandomWaypointMobilityModel",
                "Speed",
                ns3::StringValue(
                    "ns3::UniformRandomVariable[Min=8|Max=17]"),
                "Pause",
                ns3::StringValue(
                    "ns3::ConstantRandomVariable[Constant=0.5]"),
                "PositionAllocator",
                ns3::PointerValue(avPosAlloc));

            avMobility.Install(
                m_context.avs.Get(avIdx));
        }
    }

    Logger::Info("Vehicle mobility installed.");
}

};

}

#endif
