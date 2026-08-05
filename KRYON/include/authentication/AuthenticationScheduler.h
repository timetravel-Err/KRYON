#ifndef KRYON_AUTHENTICATION_SCHEDULER_H
#define KRYON_AUTHENTICATION_SCHEDULER_H

#include "../core/ExperimentConfig.h"
#include "../simulation/SimulationContext.h"
#include "../security/SecurityEngine.h"

#include "ns3/core-module.h"
#include "AuthenticationJob.h"
#include "AuthenticationPacketBuilder.h"
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
	
	m_jobsScheduled++;

	if (m_jobs.size() > m_maxQueueSize)
	{
		m_maxQueueSize = m_jobs.size();
	}

    ns3::Simulator::Schedule(
        ns3::Seconds(timeSeconds),
        &AuthenticationScheduler::RunAuthentication,
        this,
        m_jobs.size() - 1);
}


void RunAuthentication(uint32_t jobIndex)
{
    if (jobIndex >= m_jobs.size())
    {
        return;
    }

    AuthenticationJob& job = m_jobs[jobIndex];

    switch (job.currentStep)
    {
        case 0:
        {
           Logger::Info(
						"[Scheduler][" +
						job.request.requestId +
						"][Drone=" +
						std::to_string(job.request.sourceNodeId) +
						"][Vehicle=" +
						std::to_string(job.request.destinationNodeId) +
						"] RAP Step 1 : Authentication Request");
			
			AuthRequestPacket packet =
				m_packetBuilder.BuildRequest(job.request);

			Logger::Info(
				"[Scheduler] Built AuthRequestPacket (" +
				std::to_string(packet.GetPacketSize()) +
				" bytes)");

            job.state = AuthenticationState::MESSAGE1_SENT;

            job.currentStep++;

            ns3::Simulator::Schedule(
                ns3::MilliSeconds(2),
                &AuthenticationScheduler::RunAuthentication,
                this,
                jobIndex);

            break;
        }

        case 1:
        {
            Logger::Info(
							"[Scheduler][" +
							job.request.requestId +
							"] RAP Step 2 : Challenge");

            job.state = AuthenticationState::MESSAGE2_RECEIVED;

            job.currentStep++;

            ns3::Simulator::Schedule(
                ns3::MilliSeconds(2),
                &AuthenticationScheduler::RunAuthentication,
                this,
                jobIndex);

            break;
        }

        case 2:
        {
            Logger::Info(
							"[Scheduler][" +
							job.request.requestId +
							"] RAP Step 3 : Challenge Response");

            job.state = AuthenticationState::MESSAGE3_SENT;

            job.currentStep++;

            ns3::Simulator::Schedule(
                ns3::MilliSeconds(2),
                &AuthenticationScheduler::RunAuthentication,
                this,
                jobIndex);

            break;
        }

        case 3:
        {
           Logger::Info(
						"[Scheduler][" +
						job.request.requestId +
						"] RAP Step 4 : Execute Authentication");

            job.state = AuthenticationState::KEY_AGREEMENT;

            m_security.ExecuteAuthentication(job.request);

            job.currentStep++;

            ns3::Simulator::Schedule(
                ns3::MilliSeconds(1),
                &AuthenticationScheduler::RunAuthentication,
                this,
                jobIndex);

            break;
        }

        case 4:
        {
            Logger::Info(
							"[Scheduler][" +
							job.request.requestId +
							"] RAP Step 5 : Session Established");

            job.state = AuthenticationState::SESSION_ESTABLISHED;

            job.completed = true;

            job.success = true;
			
			m_jobsCompleted++;

            break;
        }

        default:
            break;
    }
}

void PrintSchedulerStatistics()
{
    kryon::Logger::Info("==========================================");
    kryon::Logger::Info("Authentication Scheduler Statistics");
    kryon::Logger::Info("==========================================");

    kryon::Logger::Info(
        "Jobs Scheduled : " +
        std::to_string(m_jobsScheduled));

    kryon::Logger::Info(
        "Jobs Completed : " +
        std::to_string(m_jobsCompleted));

    kryon::Logger::Info(
        "Maximum Queue Size : " +
        std::to_string(m_maxQueueSize));

    kryon::Logger::Info("==========================================");
}

private:

    std::vector<AuthenticationJob> m_jobs;
	
	uint32_t m_jobsScheduled = 0;

	uint32_t m_jobsCompleted = 0;

	uint32_t m_maxQueueSize = 0;

    const ExperimentConfig& m_config;

    SimulationContext& m_context;

    SecurityEngine& m_security;
	
	AuthenticationPacketBuilder m_packetBuilder;
};

}

#endif
