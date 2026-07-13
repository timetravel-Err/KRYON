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

        m_context.regionRadius =
            std::max(
                100.0,
                50.0 * std::sqrt((double) totalNodes)
            );
    }

    //--------------------------------------------------------

    void InstallDroneMobility()
    {
        //
        // Empty for now.
        //
        // We will move the UAV mobility code here
        // in Step 006.2
        //
    }

    //--------------------------------------------------------

    void InstallVehicleMobility()
    {
        //
        // Empty for now.
        //
        // We will move the AV mobility code here
        // in Step 006.3
        //
    }

};

}

#endif
