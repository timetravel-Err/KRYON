#ifndef KRYON_AUTHENTICATION_SCHEDULER_H
#define KRYON_AUTHENTICATION_SCHEDULER_H

#include "../core/ExperimentConfig.h"
#include "../simulation/SimulationContext.h"
#include "../security/SecurityEngine.h"

#include "ns3/core-module.h"
#include "AuthenticationJob.h"
#include <vector>

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
    AuthenticationJob job;

    job.request = request;

    job.state = AuthenticationState::IDLE;

    job.startTime = timeSeconds;

    job.nextEventTime = timeSeconds;

    m_jobs.push_back(job);

    ns3::Simulator::Schedule(
        ns3::Seconds(timeSeconds),
        &AuthenticationScheduler::RunAuthentication,
        this,
        m_jobs.size() - 1);
}

private:

    void RunAuthentication(uint32_t jobIndex)
{
    if (jobIndex >= m_jobs.size())
    {
        return;
    }

    AuthenticationJob& job = m_jobs[jobIndex];

    job.state = AuthenticationState::MESSAGE1_SENT;

    m_security.ExecuteAuthentication(job.request);

    job.state = AuthenticationState::SESSION_ESTABLISHED;

    job.completed = true;

    job.success = true;
}

private:

    std::vector<AuthenticationJob> m_jobs;

    const ExperimentConfig& m_config;

    SimulationContext& m_context;

    SecurityEngine& m_security;
};

}

#endif
