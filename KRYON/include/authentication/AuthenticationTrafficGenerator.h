#ifndef KRYON_AUTHENTICATION_TRAFFIC_GENERATOR_H
#define KRYON_AUTHENTICATION_TRAFFIC_GENERATOR_H

#include "../simulation/SimulationContext.h"
#include "../core/ExperimentConfig.h"
#include "../core/IdGenerator.h"
#include "AuthenticationScheduler.h"

#include "ns3/core-module.h"

namespace kryon
{

class AuthenticationTrafficGenerator
{
public:

    AuthenticationTrafficGenerator(
        const ExperimentConfig& config,
        SimulationContext& context,
        AuthenticationScheduler& scheduler)
        :
        m_config(config),
        m_context(context),
        m_scheduler(scheduler)
    {
    }

    void GenerateTraffic(double intervalSeconds)
    {
        double time = 0.0;

        for (uint32_t d = 0; d < m_context.totalDrones; ++d)
        {
            uint32_t droneId =
                m_context.drones.Get(d)->GetId();

            for (uint32_t v = 0; v < m_context.totalAVs; ++v)
            {
                AuthenticationRequest request;

                request.requestId =
                    IdGenerator::NextRequestId();

                request.sourceNodeId =
                    droneId;

                request.destinationNodeId =
                    m_context.avs.Get(v)->GetId();

                request.method =
                    AuthenticationMethod::NONE;

                request.requiresMutualAuthentication =
                    true;

                request.timestamp =
                    time;

                m_scheduler.ScheduleAuthentication(
                    request,
                    time);

                time += intervalSeconds;
            }
        }
    }

private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;

    AuthenticationScheduler& m_scheduler;
};

}

#endif
