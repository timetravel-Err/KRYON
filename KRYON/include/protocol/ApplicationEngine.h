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
 * Traffic Pattern
 * ---------------
 * • Drone → AV : 836 bytes
 * • AV → Drone : 68 bytes
 * • Authentication/application traffic uses separate ports
 *
 * Version : 0.1.0
 * ----------------------------------------------------------
 */

#include "../core/ExperimentConfig.h"
#include "../simulation/SimulationContext.h"
#include "../core/Logger.h"

#include "ns3/applications-module.h"
#include "ns3/udp-server.h"

#include <sstream>
#include <string>
#include <vector>
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

        Logger::Info(
            "[Application] Application Engine initialized.");
    }
	
	void PrintStatistics() const
    {
        uint64_t totalAvReceived = 0;
        uint32_t totalAvLost = 0;

        uint64_t totalDroneReceived = 0;
        uint32_t totalDroneLost = 0;

        for (const auto& server : m_avServers)
        {
            if (server != nullptr)
            {
                totalAvReceived += server->GetReceived();
                totalAvLost += server->GetLost();
            }
        }

        for (const auto& server : m_droneServers)
        {
            if (server != nullptr)
            {
                totalDroneReceived += server->GetReceived();
                totalDroneLost += server->GetLost();
            }
        }

        Logger::Info(
            "[Application] Drone->AV received packets = " +
            std::to_string(totalAvReceived));

        Logger::Info(
            "[Application] Drone->AV lost packets = " +
            std::to_string(totalAvLost));

        Logger::Info(
            "[Application] AV->Drone received packets = " +
            std::to_string(totalDroneReceived));

        Logger::Info(
            "[Application] AV->Drone lost packets = " +
            std::to_string(totalDroneLost));
    }

private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;
	
	std::vector<ns3::Ptr<ns3::UdpServer>> m_avServers;

    std::vector<ns3::Ptr<ns3::UdpServer>> m_droneServers;


    /*
     * ------------------------------------------------------
     * Convert ns-3 IPv4 address to string.
     * ------------------------------------------------------
     */
    std::string AddressToString(
        const ns3::Ipv4Address& address)
    {
        std::ostringstream stream;
        stream << address;
        return stream.str();
    }


    /*
     * ------------------------------------------------------
     * Install UDP servers
     * ------------------------------------------------------
     *
     * Port 9001:
     *     Drone -> AV application traffic
     *
     * Port 9002:
     *     AV -> Drone application traffic
     *
     * Port 9000 is reserved exclusively for
     * authentication traffic.
     * ------------------------------------------------------
     */
    void InstallServers()
    {
        const uint16_t requestPort = 9001;
        const uint16_t responsePort = 9002;


        /*
         * --------------------------------------------------
         * UDP servers on AVs
         * --------------------------------------------------
         */
        for (uint32_t a = 0;
             a < m_context.totalAVs;
             ++a)
        {
            ns3::Ipv4Address avIp =
                m_context.avInterfaces.GetAddress(a);

            Logger::Info(
                "[Application] Installing AV UDP server | "
                "AV=" +
                std::to_string(a) +
                " | Port=" +
                std::to_string(requestPort) +
                " | IP=" +
                AddressToString(avIp));

            ns3::UdpServerHelper avServer(requestPort);

			ns3::ApplicationContainer app =
				avServer.Install(
					m_context.avs.Get(a));

			ns3::Ptr<ns3::UdpServer> server =
				avServer.GetServer();

			m_avServers.push_back(server);

            app.Start(
                ns3::Seconds(0.0));

            app.Stop(
                ns3::Seconds(m_config.simTime));
        }


        /*
         * --------------------------------------------------
         * UDP servers on Drones
         * --------------------------------------------------
         */
        for (uint32_t d = 0;
             d < m_context.totalDrones;
             ++d)
        {
            ns3::Ipv4Address droneIp =
                m_context.droneInterfaces.GetAddress(d);

            Logger::Info(
                "[Application] Installing Drone UDP server | "
                "Drone=" +
                std::to_string(d) +
                " | Port=" +
                std::to_string(responsePort) +
                " | IP=" +
                AddressToString(droneIp));

            ns3::UdpServerHelper droneServer(
				responsePort);

			ns3::ApplicationContainer app =
				droneServer.Install(
					m_context.drones.Get(d));

			ns3::Ptr<ns3::UdpServer> server =
				droneServer.GetServer();

			m_droneServers.push_back(server);

            app.Start(
                ns3::Seconds(0.0));

            app.Stop(
                ns3::Seconds(m_config.simTime));
        }

        Logger::Info(
            "[Application] UDP servers installed.");
    }


    /*
     * ------------------------------------------------------
     * Install UDP clients
     * ------------------------------------------------------
     *
     * Drone -> AV
     *     Port       : 9001
     *     Packet     : 836 bytes
     *     Interval   : 100 ms
     *
     * AV -> Drone
     *     Port       : 9002
     *     Packet     : 68 bytes
     *     Interval   : 100 ms
     * ------------------------------------------------------
     */
    void InstallClients()
    {
        const uint16_t requestPort = 9001;
        const uint16_t responsePort = 9002;


        for (uint32_t a = 0;
             a < m_context.totalAVs;
             ++a)
        {
            /*
             * Each AV is associated with a drone.
             */
            uint32_t targetDrone =
                a % m_context.totalDrones;


            ns3::Ipv4Address avIp =
                m_context.avInterfaces.GetAddress(a);

            ns3::Ipv4Address droneIp =
                m_context.droneInterfaces.GetAddress(
                    targetDrone);


            /*
             * --------------------------------------------------
             * Drone -> AV
             * --------------------------------------------------
             */

            Logger::Info(
                "[Application] Installing Drone->AV client | "
                "Drone=" +
                std::to_string(targetDrone) +
                " | AV=" +
                std::to_string(a) +
                " | DestinationIP=" +
                AddressToString(avIp) +
                " | Port=" +
                std::to_string(requestPort) +
                " | PacketSize=836");


            ns3::UdpClientHelper droneClient(
                avIp,
                requestPort);


            droneClient.SetAttribute(
                "MaxPackets",
                ns3::UintegerValue(0));


            droneClient.SetAttribute(
                "Interval",
                ns3::TimeValue(
                    ns3::MilliSeconds(100)));


            droneClient.SetAttribute(
                "PacketSize",
                ns3::UintegerValue(836));


            ns3::ApplicationContainer droneApp =
                droneClient.Install(
                    m_context.drones.Get(targetDrone));


            double droneStart =
                10.0 + 0.1 * a;


            droneApp.Start(
                ns3::Seconds(droneStart));


            droneApp.Stop(
                ns3::Seconds(m_config.simTime));


            Logger::Info(
                "[Application] Drone->AV client installed | "
                "Start=" +
                std::to_string(droneStart) +
                " | Stop=" +
                std::to_string(m_config.simTime));


            /*
             * --------------------------------------------------
             * AV -> Drone
             * --------------------------------------------------
             */

            Logger::Info(
                "[Application] Installing AV->Drone client | "
                "AV=" +
                std::to_string(a) +
                " | Drone=" +
                std::to_string(targetDrone) +
                " | DestinationIP=" +
                AddressToString(droneIp) +
                " | Port=" +
                std::to_string(responsePort) +
                " | PacketSize=68");


            ns3::UdpClientHelper avClient(
                droneIp,
                responsePort);


            avClient.SetAttribute(
                "MaxPackets",
                ns3::UintegerValue(0));


            avClient.SetAttribute(
                "Interval",
                ns3::TimeValue(
                    ns3::MilliSeconds(100)));


            avClient.SetAttribute(
                "PacketSize",
                ns3::UintegerValue(68));


            ns3::ApplicationContainer avApp =
                avClient.Install(
                    m_context.avs.Get(a));


            double avStart =
                10.5 + 0.1 * a;


            avApp.Start(
                ns3::Seconds(avStart));


            avApp.Stop(
                ns3::Seconds(m_config.simTime));


            Logger::Info(
                "[Application] AV->Drone client installed | "
                "Start=" +
                std::to_string(avStart) +
                " | Stop=" +
                std::to_string(m_config.simTime));
        }


        Logger::Info(
            "[Application] UDP clients installed.");
    }
};

}

#endif