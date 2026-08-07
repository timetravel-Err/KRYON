#ifndef KRYON_COMMUNICATION_ENGINE_H
#define KRYON_COMMUNICATION_ENGINE_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : CommunicationEngine.h
 *
 * Description
 * -----------
 * Responsible for configuring the communication
 * infrastructure of the simulation.
 *
 * Current Support
 * ---------------
 * • Internet Stack
 * • WiFi Ad-Hoc
 * • IPv4 Assignment
 *
 * Version : 0.1.0
 * ----------------------------------------------------------
 */

#include "../core/ExperimentConfig.h"
#include "../core/Logger.h"
#include "../simulation/SimulationContext.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/olsr-module.h"
#include "ns3/wifi-module.h"
#include "ns3/socket.h"
#include "ns3/udp-socket-factory.h"

namespace kryon
{

class CommunicationEngine
{
public:

    CommunicationEngine(const ExperimentConfig& config,
                        SimulationContext& context)
        : m_config(config),
          m_context(context)
    {
    }

   void Initialize()
{
    InstallInternetStack();
	
	ConfigureWifi();

    InstallDevices();
	
	AssignIpv4Addresses();
	
	CreateSockets();
	
    Logger::Info("Communication Engine initialized.");
}

		void SendUdpPacket(
			uint32_t sourceDrone,
			uint32_t destinationVehicle,
			ns3::Ptr<ns3::Packet> packet)
		{
			m_context.droneSockets[sourceDrone]->SendTo(
				packet,
				0,
				ns3::InetSocketAddress(
					m_context.avInterfaces.GetAddress(destinationVehicle),
					9000));

			Logger::Info(
				"UDP packet sent from Drone " +
				std::to_string(sourceDrone) +
				" to Vehicle " +
				std::to_string(destinationVehicle));
		}

private:

    const ExperimentConfig& m_config;

    SimulationContext& m_context;
	
	void InstallInternetStack()
{
    ns3::OlsrHelper olsr;

    ns3::Ipv4StaticRoutingHelper staticRouting;

    ns3::Ipv4ListRoutingHelper routing;

    routing.Add(staticRouting, 0);

    routing.Add(olsr, 10);

    ns3::InternetStackHelper internet;

    internet.SetRoutingHelper(routing);

    internet.Install(m_context.drones);

    internet.Install(m_context.avs);

    Logger::Info("Internet stack installed.");
	
	
}

void ConfigureWifi()
{
    wifi.SetStandard(ns3::WIFI_STANDARD_80211n);

    wifi.SetRemoteStationManager(
        "ns3::MinstrelHtWifiManager",
        "RtsCtsThreshold",
        ns3::UintegerValue(100));

    wifiMac.SetType("ns3::AdhocWifiMac");

    wifiChannel.SetPropagationDelay(
        "ns3::ConstantSpeedPropagationDelayModel");

    wifiChannel.AddPropagationLoss(
        "ns3::FriisPropagationLossModel",
        "Frequency",
        ns3::DoubleValue(5.0e9));

    wifiPhy.SetPcapDataLinkType(
        ns3::YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

    wifiPhy.Set(
        "TxPowerStart",
        ns3::DoubleValue(23.0));

    wifiPhy.Set(
        "TxPowerEnd",
        ns3::DoubleValue(23.0));

    wifiPhy.Set(
        "RxSensitivity",
        ns3::DoubleValue(-82.0));

    ns3::Ptr<ns3::YansWifiChannel> channel =
        wifiChannel.Create();

    wifiPhy.SetChannel(channel);

    Logger::Info("WiFi configured.");
}

void InstallDevices()
{
    m_context.droneDevices =
        wifi.Install(
            wifiPhy,
            wifiMac,
            m_context.drones);

    m_context.avDevices =
        wifi.Install(
            wifiPhy,
            wifiMac,
            m_context.avs);

    Logger::Info("WiFi devices installed.");
}

void AssignIpv4Addresses()
{
    ns3::Ipv4AddressHelper ipv4;

    ipv4.SetBase(
        "10.1.0.0",
        "255.255.0.0");

    m_context.droneInterfaces =
        ipv4.Assign(m_context.droneDevices);

    m_context.avInterfaces =
        ipv4.Assign(m_context.avDevices);

    Logger::Info("IPv4 addresses assigned.");
}
void CreateSockets()
{
    //
    // Drone sockets
    //

    for (uint32_t i = 0; i < m_context.drones.GetN(); ++i)
    {
        ns3::Ptr<ns3::Socket> socket =
            ns3::Socket::CreateSocket(
                m_context.drones.Get(i),
                ns3::UdpSocketFactory::GetTypeId());

        socket->Bind(
            ns3::InetSocketAddress(
                m_context.droneInterfaces.GetAddress(i),
                9000));
				
		socket->SetRecvCallback(
		ns3::MakeCallback(
        &CommunicationEngine::ReceivePacket,
        this));		

        m_context.droneSockets.push_back(socket);
    }

    //
    // Vehicle sockets
    //

    for (uint32_t i = 0; i < m_context.avs.GetN(); ++i)
    {
        ns3::Ptr<ns3::Socket> socket =
            ns3::Socket::CreateSocket(
                m_context.avs.Get(i),
                ns3::UdpSocketFactory::GetTypeId());

        socket->Bind(
            ns3::InetSocketAddress(
                m_context.avInterfaces.GetAddress(i),
                9000));
				
		socket->SetRecvCallback(
    ns3::MakeCallback(
        &CommunicationEngine::ReceivePacket,
        this));		

        m_context.avSockets.push_back(socket);
    }

    Logger::Info("UDP sockets created.");
}

	void ReceivePacket(ns3::Ptr<ns3::Socket> socket)
{
    while (ns3::Ptr<ns3::Packet> packet = socket->Recv())
    {
        /*Logger::Info(
            "UDP packet received (" +
            std::to_string(packet->GetSize()) +
            " bytes)");*/
			
			static uint32_t counter = 0;

			counter++;

			if (counter % 100 == 0)
			{
				Logger::Info(
					"Total UDP packets received = " +
					std::to_string(counter));
			}
    }
}

void InitializeSockets()
{
    Logger::Info("Initializing UDP sockets.");
}

ns3::WifiHelper wifi;

ns3::WifiMacHelper wifiMac;

ns3::YansWifiChannelHelper wifiChannel;

ns3::YansWifiPhyHelper wifiPhy;

std::vector<ns3::Ptr<ns3::Socket>> m_droneSockets;

std::vector<ns3::Ptr<ns3::Socket>> m_vehicleSockets;

};

}

#endif
