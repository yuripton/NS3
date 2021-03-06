/* 
 * The topology used to simulate this attack contains 4 nodes as follows:
 * n0 -> enb (base station)
 * n1 -> arini (UE legitimate User)
 * a1-100 -> endro (UE attackers)
 * n2 -> gateway
 * n3 -> yuri (receiver)
     
  n0     n1  
         \
          \ pp1       
           \            
            \ n2---------n3
          ////  
         ////    
        ////pp2
       ////  
     a1...a100
*/

#include "ns3/nstime.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/mmwave-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include <ns3/buildings-helper.h>
#include "ns3/netanim-module.h"

//port TCP & UDP 5060/5061 --> widely used for SIP (Session Initiation Protocol)
#define TCP_SINK_PORT 5060
#define UDP_SINK_PORT 5061

//Experimentation parameters
#define BULK_SEND_MAX_BYTES 0 //no limit
//#define BULK_SEND_MAX_BYTES 16106127360 //15GBps
#define DDOS_RATE (std::string)"12000kb/s"

NS_LOG_COMPONENT_DEFINE ("mmwaveTCPlowRateDDoS");

//Define namespace
using namespace ns3;
using namespace mmwave;

//Main function
int main (int argc, char *argv[])
{ 

  double simTime = 10.0;
  std::string rate = "10Gbps";
  std::string delay = "5ms";
  uint32_t bots = 1;
  CommandLine cmd;
  //parameters that can be modified on simulation based on scenarios
  cmd.AddValue ("simTime", "Total duration of the simulation in second, default", simTime);
  cmd.AddValue ("bots", "Number of bots, default", bots);
  cmd.AddValue ("rate", "Data Rate of 5G mmwave, default", rate);
  cmd.AddValue ("delay", "Delay of 5G mmwave, default", delay);
  cmd.Parse (argc, argv);

  LogComponentEnable ("mmwaveTCPlowRateDDoS", LOG_INFO);

  //Creating the mmwavehelper object
  Ptr<MmWaveHelper> ptr_mmWave = CreateObject<MmWaveHelper> ();
  //and then initialising it
   ptr_mmWave->Initialize ();

  //Legitimate connection nodes 1 enB and 2 ClientServerNodes (UE)
    NodeContainer enbNodes, clientNodes, gatewayNodes, serverNodes, botNodes;
    enbNodes.Create (1);
    clientNodes.Create (1);
    serverNodes.Create (1);
    gatewayNodes.Create(1);
    botNodes.Create(bots);
  // Install Mobility Model
  MobilityHelper enbmobility;
  //set non moving enb nodes
  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
  enbPositionAlloc->Add (Vector (0.0, 0.0, 0.0)); //vector x , y, z
  enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbmobility.Install (enbNodes);

  //set non moving ue nodes
  MobilityHelper uemobility;
  Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator> ();
  uePositionAlloc->Add (Vector (10.0, 10.0, 0.0)); //vector x , y, z
  uemobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  uemobility.SetPositionAllocator (uePositionAlloc);
  uemobility.Install (clientNodes);
  
  //set non moving gateway nodes
  MobilityHelper gatewaymobility;
  Ptr<ListPositionAllocator> gatewayPositionAlloc = CreateObject<ListPositionAllocator> ();
  gatewayPositionAlloc->Add (Vector (20.0, 10.0, 0.0)); //vector x , y, z
  gatewaymobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  gatewaymobility.SetPositionAllocator (gatewayPositionAlloc);
  gatewaymobility.Install (gatewayNodes);

//set non moving server nodes
  MobilityHelper servermobility;
  Ptr<ListPositionAllocator> serverPositionAlloc = CreateObject<ListPositionAllocator> ();
  serverPositionAlloc->Add (Vector (30.0, 10.0, 0.0)); //vector x , y, z
  servermobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  servermobility.SetPositionAllocator (serverPositionAlloc);
  servermobility.Install (serverNodes);

 //set non moving bot nodes
  MobilityHelper attackermobility;
  Ptr<ListPositionAllocator> attackerPositionAlloc = CreateObject<ListPositionAllocator> ();
    uint32_t a_pos = 0;
     for (int i = 0; i < bots; ++i)
    {
        attackerPositionAlloc->Add (Vector ((double)a_pos++, 20.0, 0.0)); //vector x , y, z
        attackermobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
        attackermobility.SetPositionAllocator (attackerPositionAlloc);
        attackermobility.Install (botNodes.Get(i));
        
    }

  // Create Devices and install them in the Nodes (eNB, UE, and bots)
    NetDeviceContainer enbDevs, ueDevs, botDevs[bots];
    //NetDeviceContainer botDevs[NUMBER_OF_BOTS];

    enbDevs = ptr_mmWave->InstallEnbDevice (enbNodes);
    ueDevs = ptr_mmWave->InstallUeDevice (clientNodes);
    //botDevs = ptr_mmWave->InstallUeDevice (botNodes);
    // Activate a data radio bearer
    enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
    EpsBearer bearer (q);
      //Attach ue and bots to enb
    ptr_mmWave->AttachToClosestEnb (ueDevs, enbDevs.Get (0));
    //ptr_mmWave->AttachToClosestEnb (botDevs, enbDevs.Get (0));
    ptr_mmWave->ActivateDataRadioBearer (ueDevs, bearer);

     for (int ii = 0; ii < bots; ++ii)
    {
        botDevs[ii] = ptr_mmWave->InstallUeDevice (botNodes.Get (ii));
        //Attach ue and bots to enb
        ptr_mmWave->AttachToClosestEnb (botDevs[ii], enbDevs.Get (0));
        ptr_mmWave->ActivateDataRadioBearer (botDevs[ii], bearer);
    }


  //Define the Point-to-Point links (helpers) and their paramters
    PointToPointHelper pp1, pp2;
    pp1.SetDeviceAttribute("DataRate", StringValue(rate));
    pp1.SetChannelAttribute("Delay", StringValue(delay));

    pp2.SetDeviceAttribute("DataRate", StringValue(rate));
    pp2.SetChannelAttribute("Delay", StringValue(delay));


    // Install the Point-To-Point Connections between Nodes
    NetDeviceContainer clientGatewayDevs, clientServerDevs, botServerDevs[bots];
    clientGatewayDevs = pp1.Install(clientNodes.Get(0), gatewayNodes.Get(0));
    clientServerDevs = pp1.Install(gatewayNodes.Get(0), serverNodes.Get(0));

    for (int j = 0; j < bots; ++j)
    {
        botServerDevs[j] = pp2.Install(botNodes.Get(j), gatewayNodes.Get(0));
    }

//Use the internet stack helper and install on the ue nodes
  InternetStackHelper internet;
  internet.Install (clientNodes);
  internet.Install (serverNodes);
  internet.Install (gatewayNodes);
  internet.Install (botNodes);

  //Assigning IPv4 addresses onto the client/server nodes
  Ipv4AddressHelper ip01, ipa2, ipv4_n;
  ipv4_n.SetBase("10.10.1.0", "255.255.255.252");
  ip01.SetBase ("10.1.2.0", "255.255.255.0");
  ipa2.SetBase ("10.10.3.0", "255.255.255.0");
  Ipv4InterfaceContainer iface1 = ip01.Assign (clientGatewayDevs);
  Ipv4InterfaceContainer iface2 = ipa2.Assign (clientServerDevs);
  
    for (int jj = 0; jj < bots; ++jj)
    {
        ipv4_n.Assign(botServerDevs[jj]);
        ipv4_n.NewNetwork();
        //ipa2.NewNetwork();
    }

  // Create the OnOff applications to send TCP packets to the server
  // Create a packet sink to receive packets from OnOff application
   Address UDPSinkAddr (InetSocketAddress (iface2.GetAddress (1), UDP_SINK_PORT));
   OnOffHelper attacker ("ns3::UdpSocketFactory", UDPSinkAddr);
   attacker.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=" + std::to_string(simTime) + "]"));
   attacker.SetAttribute ("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
   attacker.SetAttribute ("DataRate", DataRateValue (DataRate (DDOS_RATE)));
   ApplicationContainer attackerApp[bots];

    //Install application in all bots
    for (int k = 0; k < bots; ++k)
    {
        attackerApp[k] = attacker.Install(botNodes.Get(k));
        attackerApp[k].Start(Seconds(0.0));
        attackerApp[k].Stop(Seconds(simTime));
    }


    Address TCPSinkAddr (InetSocketAddress (iface2.GetAddress (1), TCP_SINK_PORT));
    BulkSendHelper bulkSend("ns3::TcpSocketFactory", TCPSinkAddr);
    bulkSend.SetAttribute("MaxBytes", UintegerValue(BULK_SEND_MAX_BYTES));
    ApplicationContainer bulkSendApp = bulkSend.Install(clientNodes.Get(0));
    bulkSendApp.Start(Seconds(0.0));
    bulkSendApp.Stop(Seconds(simTime));
  
    PacketSinkHelper UDPSink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), UDP_SINK_PORT));
    ApplicationContainer UDPSinkApp = UDPSink.Install (serverNodes.Get(0));
    UDPSinkApp.Start (Seconds (0.0));
    UDPSinkApp.Stop(Seconds(simTime));

    PacketSinkHelper TCPSink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), TCP_SINK_PORT));
    ApplicationContainer TCPSinkApp = TCPSink.Install (serverNodes.Get(0));
    TCPSinkApp.Start (Seconds (0.0));
    TCPSinkApp.Stop(Seconds(simTime));

//     implement global routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

// XML output trace file, will open with NetAnim
    AnimationInterface anim("LowRateDDoSSim.xml");
    anim.SetMaxPktsPerTraceFile(99999999999999);
    ns3::AnimationInterface::SetConstantPosition(enbNodes.Get(0), 0, 0); //vector x , y, z=0
    ns3::AnimationInterface::SetConstantPosition(clientNodes.Get(0), 10, 10); //vector x , y, z=0
    ns3::AnimationInterface::SetConstantPosition(gatewayNodes.Get(0), 20, 10); //vector x , y, z=0
    ns3::AnimationInterface::SetConstantPosition(serverNodes.Get(0), 30, 10); //vector x , y, z=0
    uint32_t x_pos = 0;
    for (int l = 0; l < bots; ++l)
    {
        ns3::AnimationInterface::SetConstantPosition(botNodes.Get(l), x_pos++, 20); //vector x , y, z=0
    }

    //pcap output trace file, will open with wireshark
    pp1.EnablePcapAll("pcap");
  Simulator::Stop (Seconds (simTime));
  Simulator::Run();
  Simulator::Destroy();
  return 0;
}