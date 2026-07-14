#ifndef KRYON_APPLICATION_ENGINE_H
#define KRYON_APPLICATION_ENGINE_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : ApplicationEngine.h
 *
 * Description
 * -----------
 * Responsible for installing all application-layer
 * traffic used during the simulation.
 *
 * Current Support
 * ---------------
 * • UDP Authentication Traffic
 * • Drone → AV Requests
 * • AV → Drone Responses
 *
 * Future Support
 * --------------
 * 
 * • SLAP-IoAV
 * • Multi-hop Protocols
 * • Secure Routing
 *
 * Version : 0.1.0
 * ----------------------------------------------------------
 
	// 2PQS-IoAV Authentication & Key Agreement (AKA) traffic:
    // - Drone sends mα = ⟨EIDα, VIDα, Tα, Cα⟩ = 6688 bits (836 bytes) to AV
    // - AV replies mAV = ⟨EIDAV, SKVα−AV, TAV⟩ = 544 bits (68 bytes) to Drone
    // - Total per exchange: 7232 bits (904 bytes)
 */

#include "../core/ExperimentConfig.h"
#include "../simulation/SimulationContext.h"
#include "../core/Logger.h"
#include "ns3/applications-module.h"

namespace kryon
{

class ApplicationEngine
{
public:

    ApplicationEngine(const ExperimentConfig& config,
                      SimulationContext& context)
        : m_config(config),
          m_context(context)
    {
    }

   void Initialize()
{
    InstallServers();
	
	InstallClients();

    Logger::Info("Application Engine initialized.");
}
private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;
	
	void InstallServers()
{
    uint16_t requestPort = 9000;
    uint16_t responsePort = 9001;

    //
    // UDP Servers on AVs
    //
    for (uint32_t a = 0; a < m_context.totalAVs; ++a)
    {
        ns3::UdpServerHelper avServer(requestPort);

        ns3::ApplicationContainer app =
            avServer.Install(m_context.avs.Get(a));

        app.Start(ns3::Seconds(0.0));
        app.Stop(ns3::Seconds(m_config.simTime));
    }

    //
    // UDP Servers on Drones
    //
    for (uint32_t d = 0; d < m_context.totalDrones; ++d)
    {
        ns3::UdpServerHelper droneServer(responsePort);

        ns3::ApplicationContainer app =
            droneServer.Install(m_context.drones.Get(d));

        app.Start(ns3::Seconds(0.0));
        app.Stop(ns3::Seconds(m_config.simTime));
    }

    Logger::Info("UDP servers installed.");
}

void InstallClients()
{
    uint16_t requestPort = 9000;
    uint16_t responsePort = 9001;

    for (uint32_t a = 0; a < m_context.totalAVs; ++a)
    {
        uint32_t targetDrone =
            a % m_context.totalDrones;

        //
        // Drone → AV
        //

        ns3::UdpClientHelper droneClient(
            m_context.avInterfaces.GetAddress(a),
            requestPort);

        droneClient.SetAttribute(
            "MaxPackets",
            ns3::UintegerValue(0));

        droneClient.SetAttribute(
            "Interval",
            ns3::TimeValue(ns3::MilliSeconds(100)));

        droneClient.SetAttribute(
            "PacketSize",
            ns3::UintegerValue(836));

        ns3::ApplicationContainer droneApp =
            droneClient.Install(
                m_context.drones.Get(targetDrone));

        droneApp.Start(
            ns3::Seconds(10.0 + 0.1 * a));

        droneApp.Stop(
            ns3::Seconds(m_config.simTime));

        //
        // AV → Drone
        //

        ns3::UdpClientHelper avClient(
            m_context.droneInterfaces.GetAddress(targetDrone),
            responsePort);

        avClient.SetAttribute(
            "MaxPackets",
            ns3::UintegerValue(0));

        avClient.SetAttribute(
            "Interval",
            ns3::TimeValue(ns3::MilliSeconds(100)));

        avClient.SetAttribute(
            "PacketSize",
            ns3::UintegerValue(68));

        ns3::ApplicationContainer avApp =
            avClient.Install(
                m_context.avs.Get(a));

        avApp.Start(
            ns3::Seconds(10.5 + 0.1 * a));

        avApp.Stop(
            ns3::Seconds(m_config.simTime));
    }

    Logger::Info("UDP clients installed.");
}
};

}

#endif
