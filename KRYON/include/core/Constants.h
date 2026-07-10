#ifndef KRYON_CONSTANTS_H
#define KRYON_CONSTANTS_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 *
 * File: Constants.h
 *
 * Description:
 * Framework-wide constants used throughout KRYON.
 *
 * Version : 0.1.0
 * ----------------------------------------------------------
 */

#include <cstdint>

namespace kryon
{

class Constants
{
public:

    /* Simulation Defaults */

    static constexpr double DEFAULT_SIMULATION_TIME = 60.0;

    static constexpr uint32_t DEFAULT_NUM_REGIONS = 1;

    static constexpr uint32_t DEFAULT_DRONES_PER_REGION = 5;

    static constexpr uint32_t DEFAULT_AVS_PER_REGION = 10;

    static constexpr uint32_t DEFAULT_RANDOM_SEED = 1;

    /* Network */

    static constexpr uint32_t DEFAULT_PACKET_SIZE = 1024;

    static constexpr double DEFAULT_PACKET_INTERVAL = 1.0;

    /* Wireless */

    static constexpr double DEFAULT_TX_POWER_DBM = 20.0;

    static constexpr double DEFAULT_WIFI_RANGE_METERS = 250.0;

};

}

#endif
