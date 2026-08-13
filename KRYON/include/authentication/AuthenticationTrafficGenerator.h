#ifndef KRYON_AUTHENTICATION_TRAFFIC_GENERATOR_H
#define KRYON_AUTHENTICATION_TRAFFIC_GENERATOR_H

#include "../simulation/SimulationContext.h"
#include "../core/ExperimentConfig.h"
#include "../core/IdGenerator.h"
#include "AuthenticationScheduler.h"
#include "ns3/core-module.h"
#include "../crypto/RandomEngine.h"

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
        m_scheduler(scheduler),
		m_random(config, context)
    {
    }

    void GenerateTraffic(double intervalSeconds)
    {
		 /*
		 * Authentication start-time generation is currently
		 * randomized across the simulation.
		 *
		 * intervalSeconds is retained for the next traffic-model
		 * phase, where periodic authentication will be implemented.
		 */
		(void)intervalSeconds;

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

                double authTime =
				m_random.GenerateUniformDouble(
					0.0,
					m_config.simTime);

				request.timestamp =
					authTime;

				m_scheduler.ScheduleAuthentication(
					request,
					authTime);
            }
        }
    }

private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;

    AuthenticationScheduler& m_scheduler;
	
	RandomEngine m_random;
};

}

#endif
