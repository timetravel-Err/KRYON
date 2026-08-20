#ifndef KRYON_SESSION_MANAGER_H
#define KRYON_SESSION_MANAGER_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : SessionManager.h
 *
 * Description
 * -----------
 * Maintains all authenticated sessions established
 * between UAVs and Vehicles.
 *
 * Responsibilities
 * ----------------
 * • Create sessions
 * • Find existing sessions
 * • Find sessions by session ID
 * • Prevent duplicate active sessions
 * • Invalidate sessions
 * • Remove expired sessions
 * • Report session statistics
 * ----------------------------------------------------------
 */

#include "Session.h"

#include "../core/Logger.h"
#include "../core/ExperimentConfig.h"
#include "../simulation/SimulationContext.h"

#include <vector>
#include <algorithm>
#include <string>
#include <cstdint>
#include <stdexcept>

#include <sstream>

namespace kryon
{

class SessionManager
{
public:

    SessionManager(
        const ExperimentConfig& config,
        SimulationContext& context)
        : m_config(config),
          m_context(context)
    {
    }

    /* ------------------------------------------------------
     * Initialize
     * ------------------------------------------------------*/

    void Initialize()
    {
        Logger::Info(
            "Session Manager initialized.");
    }

	std::string GenerateSessionId(
		uint32_t droneId,
		uint32_t vehicleId,
		double currentTime)
	{
		std::ostringstream stream;

		++m_sessionSequence;

		stream
			<< "KRYON-SESSION-"
			<< droneId
			<< "-"
			<< vehicleId
			<< "-"
			<< m_sessionSequence;

		return stream.str();
	}
    /* ------------------------------------------------------
     * Finalize
     * ------------------------------------------------------*/

    void Finalize()
    {
        Logger::Info(
            "Session Manager finalized.");
    }

    /* ------------------------------------------------------
     * Create Session
     * ------------------------------------------------------*/

    void CreateSession(
        const Session& session)
    {
        /*
         * Do not allow duplicate active sessions
         * between the same Drone and Vehicle.
         */

        for (auto& existing : m_sessions)
        {
            if (existing.active &&
                existing.droneId == session.droneId &&
                existing.vehicleId == session.vehicleId)
            {
                existing = session;

                Logger::Info(
                    "Existing session replaced : " +
                    session.sessionId);

                return;
            }
        }

        m_sessions.push_back(session);

        Logger::Info(
            "Session created : " +
            session.sessionId);
    }
	
	Session CreateAuthenticatedSession(
    uint32_t droneId,
    uint32_t vehicleId,
    const SessionKey& sessionKey,
    double currentTime,
    double lifetime)
	{
		Session session;

		session.sessionId =
			GenerateSessionId(
				droneId,
				vehicleId,
				currentTime);

		session.droneId =
			droneId;

		session.vehicleId =
			vehicleId;

		session.sessionKey =
			sessionKey;

		session.creationTime =
			currentTime;

		session.expirationTime =
			currentTime + lifetime;

		session.active =
			true;

		CreateSession(session);

		return session;
	}

    /* ------------------------------------------------------
     * Find Session
     * ------------------------------------------------------*/

    Session* FindSession(
        uint32_t droneId,
        uint32_t vehicleId,
        double currentTime)
    {
        for (auto& session : m_sessions)
        {
            if (!session.active)
            {
                continue;
            }

            if (session.expirationTime <= currentTime)
            {
                continue;
            }

            if (session.droneId == droneId &&
                session.vehicleId == vehicleId)
            {
                return &session;
            }
        }

        return nullptr;
    }

    /* ------------------------------------------------------
     * Find Session By ID
     * ------------------------------------------------------*/

   Session* FindSessionById(
    const std::string& sessionId,
    double currentTime)
	{
		for (auto& session : m_sessions)
		{
			if (!session.active)
			{
				continue;
			}

			if (session.expirationTime <= currentTime)
			{
				continue;
			}

			if (session.sessionId == sessionId)
			{
				return &session;
			}
		}

		return nullptr;
	}
	
	/* ------------------------------------------------------
	 * Get Active Session
	 * ------------------------------------------------------*/

	Session* GetActiveSession(
		uint32_t droneId,
		uint32_t vehicleId,
		double currentTime)
	{
		return FindSession(
			droneId,
			vehicleId,
			currentTime);
	}

	/* ------------------------------------------------------
	 * Check Whether Session Exists and Is Valid
	 * ------------------------------------------------------*/

	bool HasValidSession(
		uint32_t droneId,
		uint32_t vehicleId,
		double currentTime)
	{
		Session* session =
			FindSession(
				droneId,
				vehicleId,
				currentTime);

		return session != nullptr;
	}


	/* ------------------------------------------------------
	 * Secure Channel Sequence Management
	 * ------------------------------------------------------*/

	/*
	 * Get and increment the next outgoing sequence number.
	 *
	 * Sequence numbers start at 1.
	 */
	uint64_t GetNextSendSequence(
		const std::string& sessionId,
		double currentTime)
	{
		Session* session =
			FindSessionById(
				sessionId,
				currentTime);

		if (session == nullptr)
		{
			throw std::runtime_error(
				"Cannot allocate sequence number: "
				"session not found or expired.");
		}

		uint64_t sequence =
			session->nextSendSequence;

		++session->nextSendSequence;

		return sequence;
	}

	/*
	 * Accept an incoming sequence number.
	 *
	 * Returns true only when the sequence number is
	 * strictly greater than the highest sequence number
	 * already accepted.
	 *
	 * IMPORTANT:
	 * The caller must invoke this only AFTER successful
	 * cryptographic authentication of the packet.
	 */
	bool AcceptReceivedSequence(
		const std::string& sessionId,
		uint64_t sequenceNumber,
		double currentTime)
	{
		Session* session =
			FindSessionById(
				sessionId,
				currentTime);

		if (session == nullptr)
		{
			return false;
		}

		if (sequenceNumber == 0)
		{
			return false;
		}

		if (sequenceNumber <=
			session->lastReceivedSequence)
		{
			return false;
		}

		session->lastReceivedSequence =
			sequenceNumber;

		return true;
	}
		/* ------------------------------------------------------
     * Invalidate Session
     * ------------------------------------------------------*/

    void InvalidateSession(
        const std::string& sessionId)
    {
        for (auto& session : m_sessions)
        {
            if (session.sessionId == sessionId)
            {
                session.active = false;

                Logger::Info(
                    "Session invalidated : " +
                    sessionId);

                return;
            }
        }

        Logger::Info(
            "Session not found for invalidation : " +
            sessionId);
    }
	
	size_t GetActiveSessionCount(
    double currentTime) const
	{
		size_t count = 0;

		for (const auto& session : m_sessions)
		{
			if (!session.active)
			{
				continue;
			}

			if (session.expirationTime <= currentTime)
			{
				continue;
			}

			++count;
		}

		return count;
	}

    /* ------------------------------------------------------
     * Remove Expired Sessions
     * ------------------------------------------------------*/

    void RemoveExpiredSessions(
        double currentTime)
    {
        auto oldSize =
            m_sessions.size();

        m_sessions.erase(

            std::remove_if(

                m_sessions.begin(),
                m_sessions.end(),

                [currentTime](
                    const Session& session)
                {
                    return
                        session.expirationTime
                        <= currentTime;
                }),

            m_sessions.end());

        auto removed =
            oldSize -
            m_sessions.size();

        if (removed > 0)
        {
            Logger::Info(
                "Expired sessions removed : " +
                std::to_string(removed));
        }
    }

private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;

    std::vector<Session> m_sessions;
	
	uint64_t m_sessionSequence = 0;
};

}

#endif