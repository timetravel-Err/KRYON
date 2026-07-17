/**
 * UAV-Assisted IoAV Communication Simulation (Enhanced ns-3.41 version)
 *
 * Improvements:
 * - UAV waypoint mobility
 * - Road-like AV mobility
 * - Realistic propagation with fading
 * - Nearest UAV association
 * - Authentication traffic model
 * - Extended metrics
 */
// Execute ./ns3 run "scratch/UAV_AV.cc --numRegions=1 --dronesPerRegion=5 --avsPerRegion=10 --run=1 --csvFile=uav_av_results.csv"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/olsr-module.h"

#include <fstream>
#include <cmath>
#include <limits>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("EnhancedDroneAVSimulation");


int main(int argc, char *argv[])
{
    uint32_t numRegions = 1;
    uint32_t dronesPerRegion = 5;
    uint32_t avsPerRegion = 10;
    uint32_t run = 1;

    double simTime = 120.0;

    std::string csvFile = "results.csv";


    CommandLine cmd;

    cmd.AddValue("numRegions",
                 "Number of regions",
                 numRegions);

    cmd.AddValue("dronesPerRegion",
                 "Number of UAVs per region",
                 dronesPerRegion);

    cmd.AddValue("avsPerRegion",
                 "Number of AVs per region",
                 avsPerRegion);

    cmd.AddValue("run",
                 "Simulation run number",
                 run);

    cmd.AddValue("csvFile",
                 "CSV output file",
                 csvFile);

    cmd.Parse(argc, argv);


    RngSeedManager::SetSeed(1);
    RngSeedManager::SetRun(run);


    uint32_t totalDrones =
        numRegions * dronesPerRegion;

    uint32_t totalAVs =
        numRegions * avsPerRegion;


    /*
     * Create nodes
     */

    NodeContainer drones;
    drones.Create(totalDrones);


    NodeContainer avs;
    avs.Create(totalAVs);



    /*
     * Internet stack with OLSR
     */

    OlsrHelper olsr;

    Ipv4StaticRoutingHelper staticRouting;

    Ipv4ListRoutingHelper routing;

    routing.Add(staticRouting, 0);
    routing.Add(olsr, 10);


    InternetStackHelper internet;

    internet.SetRoutingHelper(routing);

    internet.Install(drones);
    internet.Install(avs);



    /*
     * Mobility
     */

    double altitudeMin = 50.0;
    double altitudeMax = 150.0;


    uint32_t totalNodes =
        totalDrones + totalAVs;


    double regionRadius =
        std::max(100.0,
                 50.0 * std::sqrt((double)totalNodes));



    /*
     * UAV mobility
     *
     * UAVs follow planned patrol paths
     */

    for(uint32_t r = 0;
        r < numRegions;
        r++)
    {

        double centerX =
            r * (3 * regionRadius);


        for(uint32_t d = 0;
            d < dronesPerRegion;
            d++)
        {

            uint32_t index =
                r * dronesPerRegion + d;


            MobilityHelper mobility;


            mobility.SetMobilityModel(
                "ns3::WaypointMobilityModel"
            );


            mobility.Install(
                drones.Get(index)
            );


            Ptr<WaypointMobilityModel> model =
                drones.Get(index)
                ->GetObject<WaypointMobilityModel>();


            double offset =
                d * 25.0;


            model->AddWaypoint(
                Waypoint(
                    Seconds(0.0),
                    Vector(
                        centerX + offset,
                        0,
                        altitudeMin + offset/5
                    )
                )
            );


            model->AddWaypoint(
                Waypoint(
                    Seconds(20.0),
                    Vector(
                        centerX + regionRadius,
                        regionRadius,
                        altitudeMax
                    )
                )
            );


            model->AddWaypoint(
                Waypoint(
                    Seconds(40.0),
                    Vector(
                        centerX - regionRadius,
                        regionRadius,
                        altitudeMin
                    )
                )
            );


            model->AddWaypoint(
                Waypoint(
                    Seconds(simTime),
                    Vector(
                        centerX,
                        0,
                        altitudeMax
                    )
                )
            );

        }



        /*
         * AV mobility
         *
         * Grid/road-like movement
         */


        for(uint32_t a = 0;
            a < avsPerRegion;
            a++)
        {

            uint32_t index =
                r * avsPerRegion + a;


            MobilityHelper mobility;


            Ptr<RandomBoxPositionAllocator>
            allocator =
                CreateObject<RandomBoxPositionAllocator>();


            allocator->SetAttribute(
                "X",
                StringValue(
                "ns3::UniformRandomVariable[Min=-200|Max=200]"
                ));


            allocator->SetAttribute(
                "Y",
                StringValue(
                "ns3::UniformRandomVariable[Min=-50|Max=50]"
                ));


            allocator->SetAttribute(
                "Z",
                StringValue(
                "ns3::ConstantRandomVariable[Constant=0]"
                ));


            mobility.SetPositionAllocator(
                allocator
            );


            mobility.SetMobilityModel(
                "ns3::ConstantVelocityMobilityModel"
            );


            mobility.Install(
                avs.Get(index)
            );


            Ptr<ConstantVelocityMobilityModel>
            vehicle =
                avs.Get(index)
                ->GetObject<ConstantVelocityMobilityModel>();


            double speed =
                8.0 + (a % 10);


            vehicle->SetVelocity(
                Vector(speed,0,0)
            );
        }
    }
	
	
    /*
     * WiFi Configuration
     *
     * IEEE 802.11n Ad-Hoc
     * 5 GHz channel
     * MinstrelHT adaptive rate
     */

    WifiHelper wifi;

    wifi.SetStandard(
        WIFI_STANDARD_80211n
    );


    wifi.SetRemoteStationManager(
        "ns3::MinstrelHtWifiManager",
        "RtsCtsThreshold",
        UintegerValue(100)
    );


    WifiMacHelper wifiMac;

    wifiMac.SetType(
        "ns3::AdhocWifiMac"
    );



    /*
     * Propagation model
     *
     * Friis + LogDistance + Nakagami fading
     */

    YansWifiChannelHelper wifiChannel;


    wifiChannel.SetPropagationDelay(
        "ns3::ConstantSpeedPropagationDelayModel"
    );


    wifiChannel.AddPropagationLoss(
        "ns3::FriisPropagationLossModel",
        "Frequency",
        DoubleValue(5.0e9)
    );


  /*  wifiChannel.AddPropagationLoss(
        "ns3::LogDistancePropagationLossModel",
        "Exponent",
        DoubleValue(2.5)
    );


    wifiChannel.AddPropagationLoss(
        "ns3::NakagamiPropagationLossModel"
    );
*/

    Ptr<YansWifiChannel> channel =
        wifiChannel.Create();



    YansWifiPhyHelper wifiPhy;

    wifiPhy.SetChannel(channel);


    wifiPhy.Set(
        "TxPowerStart",
        DoubleValue(26.0)
    );


    wifiPhy.Set(
        "TxPowerEnd",
        DoubleValue(26.0)
    );


    wifiPhy.Set(
        "RxSensitivity",
        DoubleValue(-90.0)
    );



    /*
     * Install WiFi devices
     */

    NetDeviceContainer droneDevices =
        wifi.Install(
            wifiPhy,
            wifiMac,
            drones
        );


    NetDeviceContainer avDevices =
        wifi.Install(
            wifiPhy,
            wifiMac,
            avs
        );



    /*
     * IP addressing
     */

    Ipv4AddressHelper ipv4;

    ipv4.SetBase(
        "10.1.0.0",
        "255.255.0.0"
    );


    Ipv4InterfaceContainer droneInterfaces =
        ipv4.Assign(droneDevices);


    Ipv4InterfaceContainer avInterfaces =
        ipv4.Assign(avDevices);



    /*
     * Find nearest UAV for each AV
     */

    std::vector<uint32_t> avAssociation;

    for(uint32_t a = 0; a < totalAVs; a++)
{
    uint32_t drone =
        a % totalDrones;

    avAssociation.push_back(drone);
}
    for(uint32_t a = 0;
        a < totalAVs;
        a++)
    {

        Ptr<MobilityModel> avMob =
            avs.Get(a)
            ->GetObject<MobilityModel>();


        Vector avPosition =
            avMob->GetPosition();



        double minDistance =
            std::numeric_limits<double>::max();


        uint32_t selectedDrone = 0;



        for(uint32_t d = 0;
            d < totalDrones;
            d++)
        {

            Ptr<MobilityModel> droneMob =
                drones.Get(d)
                ->GetObject<MobilityModel>();


            Vector dronePosition =
                droneMob->GetPosition();


            double distance =
                CalculateDistance(
                    avPosition,
                    dronePosition
                );


            if(distance < minDistance)
            {
                minDistance = distance;
                selectedDrone = d;
            }

        }


        avAssociation.push_back(
            selectedDrone
        );
    }



    /*
     * Authentication traffic
     *
     * Drone -> AV
     * Request message = 836 bytes
     *
     * AV -> Drone
     * Response message = 68 bytes
     */


    uint16_t requestPort = 9000;

    uint16_t responsePort = 9001;



    /*
     * AV servers receive authentication requests
     */

    for(uint32_t a = 0;
        a < totalAVs;
        a++)
    {

        UdpServerHelper server(
            requestPort
        );


        ApplicationContainer app =
            server.Install(
                avs.Get(a)
            );


        app.Start(
            Seconds(0.0)
        );


        app.Stop(
            Seconds(simTime)
        );
    }



    /*
     * UAV servers receive responses
     */

    for(uint32_t d = 0;
        d < totalDrones;
        d++)
    {

        UdpServerHelper server(
            responsePort
        );


        ApplicationContainer app =
            server.Install(
                drones.Get(d)
            );


        app.Start(
            Seconds(0.0)
        );


        app.Stop(
            Seconds(simTime)
        );
    }




    /*
     * Authentication exchanges
     */

    for(uint32_t a = 0;
        a < totalAVs;
        a++)
    {

        uint32_t drone =
            avAssociation[a];



        /*
         * Drone -> AV request
         */

        UdpClientHelper request(
            avInterfaces.GetAddress(a),
            requestPort
        );


        request.SetAttribute(
            "MaxPackets",
            UintegerValue(0)
        );


        request.SetAttribute(
            "Interval",
            TimeValue(
                MilliSeconds(100)
            )
        );


        request.SetAttribute(
            "PacketSize",
            UintegerValue(836)
        );



        ApplicationContainer requestApp =
            request.Install(
                drones.Get(drone)
            );


        requestApp.Start(
            Seconds(50.0)
        );


        requestApp.Stop(
            Seconds(simTime)
        );



        /*
         * AV -> Drone response
         */

        UdpClientHelper response(
            droneInterfaces.GetAddress(drone),
            responsePort
        );


        response.SetAttribute(
            "MaxPackets",
            UintegerValue(0)
        );


        response.SetAttribute(
            "Interval",
            TimeValue(
                MilliSeconds(100)
            )
        );


        response.SetAttribute(
            "PacketSize",
            UintegerValue(68)
        );


        ApplicationContainer responseApp =
            response.Install(
                avs.Get(a)
            );


        responseApp.Start(
            Seconds(50.1)
        );


        responseApp.Stop(
            Seconds(simTime)
        );

    }
	
    /*
     * Flow Monitor
     */

    FlowMonitorHelper flowHelper;

    Ptr<FlowMonitor> monitor =
        flowHelper.InstallAll();



    Simulator::Stop(
        Seconds(simTime)
    );


    Simulator::Run();



    monitor->CheckForLostPackets();



    double totalThroughput = 0.0;
    double totalDelay = 0.0;
    double totalJitter = 0.0;

    uint64_t totalTx = 0;
    uint64_t totalRx = 0;

    uint32_t flowCount = 0;



    for(auto const &flow :
        monitor->GetFlowStats())
    {

        FlowMonitor::FlowStats stats =
            flow.second;



        totalTx += stats.txPackets;
        totalRx += stats.rxPackets;



        if(stats.rxPackets > 0)
        {

            double duration =
                stats.timeLastRxPacket.GetSeconds()
                -
                stats.timeFirstTxPacket.GetSeconds();



            if(duration > 0)
            {

                totalThroughput +=
                    (stats.rxBytes * 8.0)
                    /
                    (duration * 1000.0);

            }



            totalDelay +=
                (stats.delaySum.GetSeconds()
                *1000.0)
                /
                stats.rxPackets;



            totalJitter +=
                (stats.jitterSum.GetSeconds()
                *1000.0)
                /
                stats.rxPackets;



            flowCount++;

        }
    }



    double avgThroughput =
        (flowCount > 0)
        ?
        totalThroughput / flowCount
        :
        0.0;



    double avgDelay =
        (flowCount > 0)
        ?
        totalDelay / flowCount
        :
        0.0;



    double avgJitter =
        (flowCount > 0)
        ?
        totalJitter / flowCount
        :
        0.0;



    /*
     * Packet delivery and loss ratio
     */

    double pdr =
        (totalTx > 0)
        ?
        (double(totalRx) / totalTx)
        :
        0.0;



    double plr =
        (totalTx > 0)
        ?
        (double(totalTx - totalRx)
        /
        totalTx)
        :
        0.0;



    /*
     * CSV output
     */

    bool writeHeader =
        !std::ifstream(csvFile).good();



    std::ofstream out(
        csvFile,
        std::ios::app
    );



    if(writeHeader)
    {

        out
        << "Run,"
        << "Regions,"
        << "Drones,"
        << "AVs,"
        << "Throughput,"
        << "Delay,"
        << "Jitter,"
        << "PDR,"
        << "PLR\n";

    }



    out
    << run
    << ","
    << numRegions
    << ","
    << dronesPerRegion
    << ","
    << avsPerRegion
    << ","
    << avgThroughput
    << ","
    << avgDelay
    << ","
    << avgJitter
    << ","
    << pdr
    << ","
    << plr
    << "\n";



    out.close();


    std::cout << "TX packets: " << totalTx << std::endl;
	std::cout << "RX packets: " << totalRx << std::endl;
    Simulator::Destroy();


    return 0;

}