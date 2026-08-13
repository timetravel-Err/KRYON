#ifndef KRYON_AUTHENTICATION_SCHEDULER_H
#define KRYON_AUTHENTICATION_SCHEDULER_H

#include "../core/ExperimentConfig.h"
#include "../simulation/SimulationContext.h"
#include "../security/SecurityEngine.h"

#include "ns3/core-module.h"

#include "AuthenticationJob.h"
#include "AuthenticationPacketBuilder.h"
#include "network/AuthenticationTransport.h"
#include "AuthenticationManager.h"

#include <vector>

namespace kryon
{

class AuthenticationScheduler
{
public:

    AuthenticationScheduler(
        const ExperimentConfig& config,
        SimulationContext& context,
        SecurityEngine& security,
        AuthenticationTransport& transport)
        :
        m_config(config),
        m_context(context),
        m_security(security),
        m_transport(transport)
    {
        /*
         * AuthenticationManager notifies the scheduler when
         * the complete authentication protocol finishes.
         */
        m_security
            .GetAuthenticationManager()
            .SetCompletionHandler(
                [this](
                    const std::string& requestId,
                    bool success)
                {
                    OnAuthenticationCompleted(
                        requestId,
                        success);
                });
    }


    /*
     * ------------------------------------------------------
     * ScheduleAuthentication
     * ------------------------------------------------------
     *
     * Creates an authentication job and schedules its
     * START event.
     *
     * The scheduler does NOT execute RAP's internal
     * cryptographic steps. Those are handled by
     * AuthenticationManager / RAPAuthenticationProtocol.
     */
    void ScheduleAuthentication(
        const AuthenticationRequest& request,
        double timeSeconds)
    {
        AuthenticationJob job;

        job.request = request;

        job.state =
            AuthenticationState::IDLE;

        job.startTime =
            timeSeconds;

        job.nextEventTime =
            timeSeconds;

        job.currentStep = 0;

        job.completed = false;

        job.success = false;

        m_jobs.push_back(job);

        uint32_t jobIndex =
            static_cast<uint32_t>(m_jobs.size() - 1);

        m_jobsScheduled++;

        if (m_jobs.size() > m_maxQueueSize)
        {
            m_maxQueueSize =
                static_cast<uint32_t>(m_jobs.size());
        }

        Logger::Info(
            "[Scheduler][" +
            request.requestId +
            "] Authentication scheduled at t=" +
            std::to_string(timeSeconds) +
            " s");

        ns3::Simulator::Schedule(
            ns3::Seconds(timeSeconds),
            &AuthenticationScheduler::RunAuthentication,
            this,
            jobIndex);
    }


    /*
     * ------------------------------------------------------
     * RunAuthentication
     * ------------------------------------------------------
     *
     * Starts the authentication transaction.
     *
     * Only Message 1 is generated here.
     *
     * RAP itself performs the remaining authentication
     * operations after the request reaches the receiver.
     */
    void RunAuthentication(
        uint32_t jobIndex)
    {
        if (jobIndex >= m_jobs.size())
        {
            Logger::Warning(
                "[Scheduler] Invalid authentication job index.");
            return;
        }

        AuthenticationJob& job =
            m_jobs[jobIndex];

        if (job.completed)
        {
            return;
        }

        /*
         * Only start an IDLE job.
         */
        if (job.state != AuthenticationState::IDLE)
        {
            return;
        }

        Logger::Info(
            "[Scheduler][" +
            job.request.requestId +
            "][Drone=" +
            std::to_string(
                job.request.sourceNodeId) +
            "][Vehicle=" +
            std::to_string(
                job.request.destinationNodeId) +
            "] Starting authentication");

        /*
         * Build first authentication packet.
         */
        AuthRequestPacket packet =
            m_packetBuilder.BuildRequest(
                job.request);

        Logger::Info(
            "[Scheduler][" +
            job.request.requestId +
            "] AuthRequestPacket size = " +
            std::to_string(
                packet.GetPacketSize()) +
            " bytes");

        /*
         * Send Message 1 through the actual transport.
         */
        m_transport.SendRequest(packet);

        /*
         * Update scheduler state.
         */
        job.state =
            AuthenticationState::MESSAGE1_SENT;

        job.currentStep = 1;

        job.nextEventTime =
            ns3::Simulator::Now().GetSeconds();

        Logger::Info(
            "[Scheduler][" +
            job.request.requestId +
            "] Authentication request transmitted.");
    }


    /*
     * ------------------------------------------------------
     * OnAuthenticationCompleted
     * ------------------------------------------------------
     *
     * Called by AuthenticationManager after RAP has
     * completed.
     */
    void OnAuthenticationCompleted(
        const std::string& requestId,
        bool success)
    {
        for (auto& job : m_jobs)
        {
            if (job.request.requestId != requestId)
            {
                continue;
            }

            /*
             * Prevent duplicate completion callbacks.
             */
            if (job.completed)
            {
                Logger::Warning(
                    "[Scheduler][" +
                    requestId +
                    "] Duplicate completion ignored.");

                return;
            }

            job.completed = true;

            job.success = success;

            if (success)
            {
                job.state =
                    AuthenticationState::SESSION_ESTABLISHED;

                job.currentStep =
                    job.totalSteps;
            }
            else
            {
                job.state =
                    AuthenticationState::FAILED;
            }

            job.nextEventTime =
                ns3::Simulator::Now().GetSeconds();

            m_jobsCompleted++;

            Logger::Info(
                "[Scheduler][" +
                requestId +
                "] Authentication completed : " +
                (success ? "SUCCESS" : "FAILURE"));

            return;
        }

        Logger::Warning(
            "[Scheduler] Completion received for unknown request : " +
            requestId);
    }


    /*
     * ------------------------------------------------------
     * Statistics
     * ------------------------------------------------------
     */
    void PrintSchedulerStatistics()
    {
        Logger::Info(
            "==========================================");

        Logger::Info(
            "Authentication Scheduler Statistics");

        Logger::Info(
            "==========================================");

        Logger::Info(
            "Jobs Scheduled : " +
            std::to_string(m_jobsScheduled));

        Logger::Info(
            "Jobs Completed : " +
            std::to_string(m_jobsCompleted));

        Logger::Info(
            "Jobs Pending : " +
            std::to_string(
                m_jobsScheduled - m_jobsCompleted));

        Logger::Info(
            "Maximum Queue Size : " +
            std::to_string(m_maxQueueSize));

        Logger::Info(
            "==========================================");
    }


private:

    std::vector<AuthenticationJob> m_jobs;

    uint32_t m_jobsScheduled = 0;

    uint32_t m_jobsCompleted = 0;

    uint32_t m_maxQueueSize = 0;

    const ExperimentConfig& m_config;

    SimulationContext& m_context;

    SecurityEngine& m_security;

    AuthenticationTransport& m_transport;

    AuthenticationPacketBuilder m_packetBuilder;
};

}

#endif