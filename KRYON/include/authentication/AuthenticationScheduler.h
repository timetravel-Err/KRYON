#ifndef KRYON_AUTHENTICATION_SCHEDULER_H
#define KRYON_AUTHENTICATION_SCHEDULER_H

#include "../core/ExperimentConfig.h"
#include "../simulation/SimulationContext.h"
#include "../security/SecurityEngine.h"

#include "ns3/core-module.h"

namespace kryon
{

class AuthenticationScheduler
{
public:

    AuthenticationScheduler(
        const ExperimentConfig& config,
        SimulationContext& context,
        SecurityEngine& security)
        :
        m_config(config),
        m_context(context),
        m_security(security)
    {
    }

    void ScheduleAuthentication(
        const AuthenticationRequest& request,
        double timeSeconds)
    {
        ns3::Simulator::Schedule(
            ns3::Seconds(timeSeconds),
            &AuthenticationScheduler::RunAuthentication,
            this,
            request);
    }

private:

    void RunAuthentication(
        AuthenticationRequest request)
    {
        m_security.ExecuteAuthentication(request);
    }

private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;

    SecurityEngine& m_security;
};

}

#endif
