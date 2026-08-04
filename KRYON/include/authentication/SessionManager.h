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

namespace kryon
{

class SessionManager
{
public:

    SessionManager(const ExperimentConfig& config,
                   SimulationContext& context)
        : m_config(config),
          m_context(context)
    {
    }

    void Initialize()
    {
        Logger::Info("Session Manager initialized.");
    }

    void Finalize()
    {
        Logger::Info("Session Manager finalized.");
    }

    void CreateSession(const Session& session)
    {
        m_sessions.push_back(session);

        Logger::Info(
            "Session created : " +
            session.sessionId);
    }

    Session* FindSession(uint32_t droneId,
                         uint32_t vehicleId)
    {
        for (auto& session : m_sessions)
        {
            if (session.active &&
                session.droneId == droneId &&
                session.vehicleId == vehicleId)
            {
                return &session;
            }
        }

        return nullptr;
    }

    void RemoveExpiredSessions(double currentTime)
    {
        m_sessions.erase(

            std::remove_if(

                m_sessions.begin(),
                m_sessions.end(),

                [currentTime](const Session& session)
                {
                    return session.expirationTime <= currentTime;
                }),

            m_sessions.end());
    }

    uint32_t GetActiveSessionCount() const
    {
        return static_cast<uint32_t>(m_sessions.size());
    }

private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;

    std::vector<Session> m_sessions;

};

}

#endif
