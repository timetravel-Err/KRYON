#ifndef KRYON_SIMULATION_CONTEXT_H
#define KRYON_SIMULATION_CONTEXT_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : SimulationContext.h
 *
 * Description:
 * Shared simulation state passed between all framework
 * components. This avoids long parameter lists and provides
 * a common interface for managers.
 * ----------------------------------------------------------
 */

#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"

namespace kryon
{

struct SimulationContext
{
    /* ---------- Nodes ---------- */

    ns3::NodeContainer drones;
    ns3::NodeContainer avs;
	
	/* ---------- Topology ---------- */

	uint32_t totalDrones = 0;
	uint32_t totalAVs = 0;
	
	/* ---------- Region Geometry ---------- */

	double regionRadius = 0.0;

    /* ---------- Mobility ---------- */

	double droneAltitudeMin = 50.0;
	double droneAltitudeMax = 150.0;
    
	/* ---------- Devices ---------- */

    ns3::NetDeviceContainer droneDevices;
    ns3::NetDeviceContainer avDevices;

    /* ---------- IP Interfaces ---------- */

    ns3::Ipv4InterfaceContainer droneInterfaces;
    ns3::Ipv4InterfaceContainer avInterfaces;

    /* ---------- Monitoring ---------- */

    ns3::Ptr<ns3::FlowMonitor> monitor;
	/* ---------- Performance Metrics ---------- */

	double throughput = 0.0;
	double delay = 0.0;
	double jitter = 0.0;
	double pdr = 0.0;

};

}

#endif
