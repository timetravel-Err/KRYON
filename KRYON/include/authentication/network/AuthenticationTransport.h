#ifndef KRYON_AUTHENTICATION_TRANSPORT_H
#define KRYON_AUTHENTICATION_TRANSPORT_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : AuthenticationTransport.h
 *
 * Description
 * -----------
 * Simulates transmission of authentication packets through
 * the ns-3 network.
 *
 * Current Version
 * ---------------
 * - Schedules packet delivery events
 * - Logs packet transmission
 * - Ready for future UDP socket integration
 *
 * Future Version
 * --------------
 * - Real UDP socket communication
 * - Packet loss simulation
 * - Wireless propagation delay
 * - Multi-hop forwarding
 * ----------------------------------------------------------
 */

#include "../../core/ExperimentConfig.h"
#include "../../simulation/SimulationContext.h"
#include "../../core/Logger.h"

#include "../packets/AuthRequestPacket.h"
#include "../packets/AuthChallengePacket.h"
#include "../packets/AuthResponsePacket.h"
#include "../packets/AuthConfirmPacket.h"
#include "AuthenticationReceiver.h"

#include "ns3/core-module.h"

#include <functional>

namespace kryon
{

class AuthenticationTransport
{
public:

    using RequestHandler =
        std::function<void(const AuthRequestPacket&)>;

    AuthenticationTransport(
        const ExperimentConfig& config,
        SimulationContext& context)
        :
        m_config(config),
        m_context(context)
    {
    }

    /* ------------------------------------------------------
     * Initialize
     * ------------------------------------------------------*/

    void Initialize()
    {
        Logger::Info("Authentication Transport initialized.");
    }

    /* ------------------------------------------------------
     * Register Receiver
     * ------------------------------------------------------*/

    void RegisterRequestHandler(
        RequestHandler handler)
    {
        m_requestHandler = handler;
    }

    /* ------------------------------------------------------
     * Send Request Packet
     * ------------------------------------------------------*/

    void SendRequest(
        const AuthRequestPacket& packet,
        double delayMs = 1.0)
    {
        Logger::Info(
            "[Transport] Sending AuthRequestPacket (" +
            packet.requestId +
            ") from Drone " +
            std::to_string(packet.sourceNode) +
            " to Vehicle " +
            std::to_string(packet.destinationNode));

        ns3::Simulator::Schedule(
            ns3::MilliSeconds(delayMs),
            &AuthenticationTransport::DeliverRequest,
            this,
            packet);
    }

    /* ------------------------------------------------------
     * Finalize
     * ------------------------------------------------------*/

    void Finalize()
    {
        Logger::Info("Authentication Transport finalized.");
    }

private:

  void DeliverRequest(AuthRequestPacket packet)
{
    Logger::Info(
        "[Transport] Delivered AuthRequestPacket (" +
        packet.requestId + ")");

    if (!m_requestHandler)
    {
        Logger::Warning(
            "[Transport] No RequestHandler registered.");
        return;
    }

    m_requestHandler(packet);
}

private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;

    RequestHandler m_requestHandler;
	

};

}

#endif
