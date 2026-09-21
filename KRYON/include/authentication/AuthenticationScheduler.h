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
#include <algorithm>
#include <cmath>
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
		 * Record the actual simulation time at which
		 * authentication starts.
		 */
		job.startTime =
			ns3::Simulator::Now().GetSeconds();
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
			
			/*
			 * Calculate authentication latency.
			 *
			 * Latency is measured from the actual authentication
			 * start time until the completion callback is received.
			 */
			job.authenticationLatency =
				ns3::Simulator::Now().GetSeconds() -
				job.startTime;
				
			/*
			 * Store the measured latency for aggregate
			 * authentication-latency statistics.
			 */
			m_authenticationLatencies.push_back(
				job.authenticationLatency);	

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
				
				Logger::Info(
				"[Scheduler][" +
				requestId +
				"] Authentication latency = " +
				std::to_string(
					job.authenticationLatency) +
				" s");

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

    /*
     * --------------------------------------------------
     * Authentication Latency Statistics
     * --------------------------------------------------
     *
     * Latency is measured from the actual authentication
     * start time until the completion callback is received.
     */
    Logger::Info(
        "==========================================");

    Logger::Info(
        "Authentication Latency Statistics");

    Logger::Info(
        "==========================================");

    if (m_authenticationLatencies.empty())
    {
        Logger::Info(
            "Latency Measurements : 0");

        Logger::Info(
            "==========================================");

        return;
    }

    /*
     * Number of latency measurements.
     */
    const size_t sampleCount =
        m_authenticationLatencies.size();

    /*
     * Calculate minimum, maximum and mean.
     */
    double sum = 0.0;

    double minimum =
        m_authenticationLatencies[0];

    double maximum =
        m_authenticationLatencies[0];

    for (double latency :
         m_authenticationLatencies)
    {
        sum += latency;

        if (latency < minimum)
        {
            minimum = latency;
        }

        if (latency > maximum)
        {
            maximum = latency;
        }
    }

    double mean =
        sum /
        static_cast<double>(sampleCount);

    /*
     * Create a sorted copy so the original
     * measurement order is preserved.
     */
    std::vector<double> sortedLatencies =
        m_authenticationLatencies;

    std::sort(
        sortedLatencies.begin(),
        sortedLatencies.end());

    /*
     * Calculate median.
     */
    double median = 0.0;

    if (sampleCount % 2 == 0)
    {
        median =
            (
                sortedLatencies[
                    sampleCount / 2 - 1] +
                sortedLatencies[
                    sampleCount / 2]
            ) / 2.0;
    }
    else
    {
        median =
            sortedLatencies[
                sampleCount / 2];
    }

    /*
     * Calculate population standard deviation.
     *
     * This describes the dispersion of the observed
     * authentication transactions in this simulation run.
     */
    double squaredDifferenceSum = 0.0;

    for (double latency :
         m_authenticationLatencies)
    {
        double difference =
            latency - mean;

        squaredDifferenceSum +=
            difference * difference;
    }

    double standardDeviation =
        std::sqrt(
            squaredDifferenceSum /
            static_cast<double>(sampleCount));

    /*
     * Calculate the 95th percentile using
     * linear interpolation.
     *
     * Position = (N - 1) * 0.95
     */
    double percentilePosition =
        (static_cast<double>(sampleCount) - 1.0)
        * 0.95;

    size_t lowerIndex =
        static_cast<size_t>(
            std::floor(percentilePosition));

    size_t upperIndex =
        static_cast<size_t>(
            std::ceil(percentilePosition));

    double percentile95 = 0.0;

    if (lowerIndex == upperIndex)
    {
        percentile95 =
            sortedLatencies[lowerIndex];
    }
    else
    {
        double fraction =
            percentilePosition -
            static_cast<double>(lowerIndex);

        percentile95 =
            sortedLatencies[lowerIndex] +
            fraction *
            (
                sortedLatencies[upperIndex] -
                sortedLatencies[lowerIndex]
            );
    }

    /*
     * Print measurements in milliseconds.
     *
     * Internal storage remains in seconds.
     */
    Logger::Info(
        "Latency Measurements : " +
        std::to_string(sampleCount));

    Logger::Info(
        "Mean Latency : " +
        std::to_string(mean * 1000.0) +
        " ms");

    Logger::Info(
        "Minimum Latency : " +
        std::to_string(minimum * 1000.0) +
        " ms");

    Logger::Info(
        "Maximum Latency : " +
        std::to_string(maximum * 1000.0) +
        " ms");

    Logger::Info(
        "Median Latency : " +
        std::to_string(median * 1000.0) +
        " ms");

    Logger::Info(
        "Standard Deviation : " +
        std::to_string(
            standardDeviation * 1000.0) +
        " ms");

    Logger::Info(
        "95th Percentile Latency : " +
        std::to_string(
            percentile95 * 1000.0) +
        " ms");

    Logger::Info(
        "==========================================");
}

private:

    std::vector<AuthenticationJob> m_jobs;
	
	/*
	 * Stores authentication latency measurements
	 * for completed authentication transactions.
	 */
	std::vector<double> m_authenticationLatencies;

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