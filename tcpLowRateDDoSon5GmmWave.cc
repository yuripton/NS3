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
        ////pp1
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

//port TCP & UDP 5060/5061 --> widely used for SIP (Session Initiation Protocol)
#define TCP_SINK_PORT 5060
#define UDP_SINK_PORT 5061

//Experimentation parameters
#define BULK_SEND_MAX_BYTES 2097152
#define MAX_SIMULATION_TIME 240.0
#define ATTACKER_START 0.0
#define ATTACKER_RATE (std::string)"12000kb/s"
//Number of Bots for DDoS
#define NUMBER_OF_BOTS 4


//Main function
int main (int argc, char *argv[])
{
  //mmwave module for 5G
    Ptr<MmWaveHelper> mmwaveHelper = CreateObject<MmWaveHelper> ();
  //and then initialising it
   ptr_mmWave->Initialize ();

  //Legitimate connection nodes 1 enB and 2 ClientServerNodes (UE)
    NodeContainer enbNodes;
    NodeContainer clientServerNodes;
    enbNodes.Create (1);
    clientServerNodes.Create (2);

  //Nodes for attacker bots
    NodeContainer botNodes;
    botNodes.Create(NUMBER_OF_BOTS);

  // Install Mobility Model
    MobilityHelper mobility;
  //set non moving enb nodes
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (enbNodes);
    BuildingsHelper::Install (enbNodes);
  //set non moving ue nodes
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (clientServerNodes.Get(0));
    BuildingsHelper::Install (clientServerNodes.Get(0));
     for (int i = 0; i < NUMBER_OF_BOTS; ++i)
    {
        mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
        mobility.Install (botNodes.Get(i));
        
    }

  // Create Devices and install them in the Nodes (eNB, UE, and bots)
    NetDeviceContainer enbDevs;
    NetDeviceContainer ueDevs;
    NetDeviceContainer botDevs[NUMBER_OF_BOTS];

    enbDevs = ptr_mmWave->InstallEnbDevice (enbNodes);
    ueDevs = ptr_mmWave->InstallUeDevice (clientServerNodes.Get (0));

    //Attach ue and bots to enb
    ptr_mmWave->AttachToClosestEnb (ueDevs, enbDevs.Get (0));

     for (int ii = 0; ii < NUMBER_OF_BOTS; ++ii)
    {
        botDevs[ii] = ptr_mmWave->InstallUeDevice (botNodes.Get (ii));
        //Attach ue and bots to enb
        ptr_mmWave->AttachToClosestEnb (botDevs[ii], enbDevs.Get (0));
    }
 
  //Define the Point-to-Point links (helpers) and their paramters
    PointToPointHelper pp1, pp2;
    pp1.SetDeviceAttribute("DataRate", StringValue("10Gbps"));
    pp1.SetChannelAttribute("Delay", StringValue("1ms"));

    pp2.SetDeviceAttribute("DataRate", StringValue("10Gbps"));
    pp2.SetChannelAttribute("Delay", StringValue("1ms"));


    // Install the Point-To-Point Connections between Nodes
    NetDeviceContainer clientServerDevs;
    clientServerDevs = pp1.Install(clientServerNodes);

    for (int i = 0; i < NUMBER_OF_BOTS; ++i)
    {
        botDevs[i] = pp2.Install(botNodes.Get(i));
    }

//Use the internet stack helper and install on the ue nodes
  InternetStackHelper internet;
  internet.Install (clientServerNodes);
  
  //Assigning IPv4 addresses onto the clien/server nodes
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (clientServerDevs);

    for (int j = 0; j < NUMBER_OF_BOTS; ++j)
    {
        ipv4_n.Assign(botDevs[j]);
        ipv4_n.NewNetwork();
    }

  Simulator::Run();
  Simulator::Destroy();
  return 0;
}