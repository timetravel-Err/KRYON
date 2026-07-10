#ifndef KRYON_EXPERIMENT_CONFIG_H
#define KRYON_EXPERIMENT_CONFIG_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File: ExperimentConfig.h
 *
 * Description:
 * Stores all experiment parameters and parses
 * command-line arguments.
 *
 * ----------------------------------------------------------
 */

#include <string>

#include "ns3/core-module.h"

namespace kryon
{

class ExperimentConfig
{
public:

    /* Simulation Parameters */

    uint32_t numRegions = 1;

    uint32_t dronesPerRegion = 5;

    uint32_t avsPerRegion = 10;

    uint32_t run = 1;

    double simTime = 60.0;

    std::string csvFile = "scratch/results.csv";

    void Parse(int argc, char *argv[])
    {
        ns3::CommandLine cmd(__FILE__);

        cmd.AddValue("numRegions",
                     "Number of regions",
                     numRegions);

        cmd.AddValue("dronesPerRegion",
                     "Drones per region",
                     dronesPerRegion);

        cmd.AddValue("avsPerRegion",
                     "AVs per region",
                     avsPerRegion);

        cmd.AddValue("run",
                     "Random seed",
                     run);

        cmd.AddValue("simTime",
                     "Simulation time",
                     simTime);

        cmd.AddValue("csvFile",
                     "CSV output file",
                     csvFile);

        cmd.Parse(argc, argv);
    }
};

}

#endif
