/* 
 * The topology used to simulate this attack contains 7 nodes as follows:
 * n0 -> arini (UE legitimate User)
 * a1,a2,a3,a4 -> endro (4 UE attackers)
 * n1 -> enb (base station connected to yuri)
 * n2 -> yuri (receiver)
     n0
        \ pp1 
         \
          \       
           \            |
            n1 ---- n2
          ////  
         ////    
        ////pp2
       ////  
     a1a2a3a4
*/

#include "ns3/nstime.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/mmwave-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include <ns3/buildings-helper.h>

//port TCP & UDP 5060/5061 --> widely used for SIP (Session Initiation Protocol)
#define TCP_SINK_PORT 5060
#define UDP_SINK_PORT 5061

//Experimentation parameters
#define BULK_SEND_MAX_BYTES 2097152
#define MAX_SIMULATION_TIME 10.0
#define ATTACKER_START 0.0
#define ON_TIME (std::string)"0.25"
#define ATTACKER_RATE (std::string)"12000kb/s"
#define BURST_PERIOD 1
#define OFF_TIME std::to_string(BURST_PERIOD - stof(ON_TIME))
#define SENDER_START 0.75 // Must be equal to OFF_TIME
//Number of Bots for DDoS
#define NUMBER_OF_BOTS 4

NS_LOG_COMPONENT_DEFINE ("mmwaveTCPlowRateDDoS");

//Define namespace
using namespace ns3;
using namespace mmwave;

//Main function
int main (int argc, char *argv[])
{ 
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);

  LogComponentEnable ("mmwaveTCPlowRateDDoS", LOG_INFO);
  //Creating the mmwavehelper object
  Ptr<MmWaveHelper> ptr_mmWave = CreateObject<MmWaveHelper> ();
  //and then initialising it
   ptr_mmWave->Initialize ();

  //Legitimate connection nodes 1 enB and 2 ClientServerNodes (UE)
    NodeContainer enbNodes, clientServerNodes, botNodes;
    enbNodes.Create (1);
    clientServerNodes.Create (2);
    botNodes.Create(1);
  // //Nodes for attacker bots
  //   NodeContainer botNodes;
  //   botNodes.Create(NUMBER_OF_BOTS);

  // // Install Mobility Model
  //   MobilityHelper mobility;
  // //set non moving enb nodes
  //   mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  //   mobility.Install (enbNodes);
  //   BuildingsHelper::Install (enbNodes);
  // //set non moving ue nodes
  //   mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  //   mobility.Install (clientServerNodes.Get(0));
  //   BuildingsHelper::Install (clientServerNodes.Get(0));
      // Install Mobility Model
  MobilityHelper enbmobility;
    //set non moving enb nodes
  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
  enbPositionAlloc->Add (Vector (0.0, 0.0, 0.0));
  enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbmobility.Install (enbNodes);
    //set randomly walking ue nodes
 MobilityHelper uemobility;
  Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator> ();
  uePositionAlloc->Add (Vector (30.0, 0.0, 0.0));
  uemobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  uemobility.SetPositionAllocator (uePositionAlloc);
  uemobility.Install (clientServerNodes.Get(0));
  
  MobilityHelper attackermobility;
  Ptr<ListPositionAllocator> attackerPositionAlloc = CreateObject<ListPositionAllocator> ();
  attackerPositionAlloc->Add (Vector (40.0, 0.0, 0.0));
  attackermobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  attackermobility.SetPositionAllocator (attackerPositionAlloc);
  attackermobility.Install (botNodes);
    //  for (int i = 0; i < NUMBER_OF_BOTS; ++i)
    // {
    //     mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    //     mobility.Install (botNodes.Get(i));
    //     BuildingsHelper::Install (botNodes.Get(i));
        
    // }

  // Create Devices and install them in the Nodes (eNB, UE, and bots)
    NetDeviceContainer enbDevs, ueDevs, botDevs;
    //NetDeviceContainer botDevs[NUMBER_OF_BOTS];

    enbDevs = ptr_mmWave->InstallEnbDevice (enbNodes);
    ueDevs = ptr_mmWave->InstallUeDevice (clientServerNodes.Get (0));
    botDevs = ptr_mmWave->InstallUeDevice (botNodes);
    //  for (int ii = 0; ii < NUMBER_OF_BOTS; ++ii)
    // {
    //     botDevs[ii] = ptr_mmWave->InstallUeDevice (botNodes.Get (ii));
    //     //Attach ue and bots to enb
    //     ptr_mmWave->AttachToClosestEnb (botDevs[ii], enbDevs.Get (0));
    // }
    //Attach ue and bots to enb
    ptr_mmWave->AttachToClosestEnb (ueDevs, enbDevs.Get (0));
    ptr_mmWave->AttachToClosestEnb (botDevs, enbDevs.Get (0));
  
  //Define the Point-to-Point links (helpers) and their paramters
    PointToPointHelper pp1, pp2;
    pp1.SetDeviceAttribute("DataRate", StringValue("10Gbps"));
    pp1.SetChannelAttribute("Delay", StringValue("5ms"));

    // pp2.SetDeviceAttribute("DataRate", StringValue("10Gbps"));
    // pp2.SetChannelAttribute("Delay", StringValue("5ms"));


    // Install the Point-To-Point Connections between Nodes
    NetDeviceContainer clientServerDevs, botServerDevs;
    clientServerDevs = pp1.Install(clientServerNodes);
    botServerDevs = pp1.Install(botNodes.Get(0), clientServerNodes.Get(1));

    // for (int i = 0; i < NUMBER_OF_BOTS; ++i)
    // {
    //     botDevs[i] = pp2.Install(botNodes.Get(i));
    // }

//Use the internet stack helper and install on the ue nodes
  InternetStackHelper internet;
  internet.Install (clientServerNodes);
  internet.Install (botNodes);
  
  //Assigning IPv4 addresses onto the clien/server nodes
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.10.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iface1 = ipv4.Assign (clientServerDevs);
  Ipv4InterfaceContainer iface2 = ipv4.Assign (botServerDevs);
  //ipv4.NewNetwork();
    // for (int j = 0; j < NUMBER_OF_BOTS; ++j)
    // {
    //     ipv4.Assign(botDevs[j]);
    //     ipv4.NewNetwork();
    // }

  // Create a packet sink to receive packets from OnOff application
  Address UDPSinkAddr (InetSocketAddress (iface1.GetAddress (1), UDP_SINK_PORT));
  PacketSinkHelper UDPSink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), UDP_SINK_PORT));
  ApplicationContainer UDPSinkApp = UDPSink.Install (clientServerNodes.Get(1));
  UDPSinkApp.Start (Seconds (0.0));
  UDPSinkApp.Stop(Seconds(MAX_SIMULATION_TIME));

  Address TCPSinkAddr (InetSocketAddress (iface1.GetAddress (1), TCP_SINK_PORT));
  PacketSinkHelper TCPSink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), TCP_SINK_PORT));
  ApplicationContainer TCPSinkApp = TCPSink.Install (clientServerNodes.Get(1));
  TCPSinkApp.Start (Seconds (0.0));
  TCPSinkApp.Stop(Seconds(MAX_SIMULATION_TIME));

  // Create the OnOff applications to send TCP packets to the server
   //uint32_t MaxPacketSize = 1024; //max packet size to send in bytes
   OnOffHelper attacker ("ns3::UdpSocketFactory", UDPSinkAddr);
   attacker.SetAttribute ("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=" + ON_TIME + "]"));
   attacker.SetAttribute ("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=" + OFF_TIME + "]"));
   attacker.SetAttribute ("DataRate", DataRateValue (DataRate (ATTACKER_RATE)));
   //client.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
   attacker.SetAttribute ("MaxBytes", UintegerValue (BULK_SEND_MAX_BYTES));
   ApplicationContainer attackerApp = attacker.Install (botNodes.Get(0));
   attackerApp.Start (Seconds (ATTACKER_START));
   attackerApp.Stop(Seconds(MAX_SIMULATION_TIME));


// // DDoS Application Behaviour
//   OnOffHelper onoff("ns3::UdpSocketFactory", Address(InetSocketAddress(iface.GetAddress(2), UDP_SINK_PORT)));
//   onoff.SetConstantRate(DataRate(ATTACKER_RATE));
//   onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=" + ON_TIME + "]"));
//   onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=" + OFF_TIME + "]"));
//   ApplicationContainer onOffApp = onoff.Install(clientServerNodes.Get(2));
//   onOffApp.Start(Seconds(ATTACKER_START));
//   onOffApp.Stop(Seconds(MAX_SIMULATION_TIME));

//     // Sender Application (Packets generated by this application are throttled)
    BulkSendHelper bulkSend("ns3::TcpSocketFactory", TCPSinkAddr);
    bulkSend.SetAttribute("MaxBytes", UintegerValue(BULK_SEND_MAX_BYTES));
    ApplicationContainer bulkSendApp = bulkSend.Install(clientServerNodes.Get(0));
    bulkSendApp.Start(Seconds(SENDER_START));
    bulkSendApp.Stop(Seconds(MAX_SIMULATION_TIME));

//     // UDPSink on receiver side
//     PacketSinkHelper UDPsink("ns3::UdpSocketFactory",
//                              Address(InetSocketAddress(Ipv4Address::GetAny(), UDP_SINK_PORT)));
//     ApplicationContainer UDPSinkApp = UDPsink.Install(clientServerNodes.Get(2));
//     UDPSinkApp.Start(Seconds(0.0));
//     UDPSinkApp.Stop(Seconds(MAX_SIMULATION_TIME));

//     // TCP Sink Application on server side
//     PacketSinkHelper TCPsink("ns3::TcpSocketFactory",
//                              InetSocketAddress(Ipv4Address::GetAny(), TCP_SINK_PORT));
//     ApplicationContainer TCPSinkApp = TCPsink.Install(clientServerNodes.Get(1));
//     TCPSinkApp.Start(Seconds(0.0));
//     TCPSinkApp.Stop(Seconds(MAX_SIMULATION_TIME));
    pp1.EnablePcapAll("pcap1");
    //pp2.EnablePcapAll("pcap2");
  Simulator::Stop (Seconds (MAX_SIMULATION_TIME));
  Simulator::Run();
  Simulator::Destroy();
  return 0;
}