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
	
    Logger::Info("Communication Engine initialized.");
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

ns3::WifiHelper wifi;

ns3::WifiMacHelper wifiMac;

ns3::YansWifiChannelHelper wifiChannel;

ns3::YansWifiPhyHelper wifiPhy;
};

}

#endif
