/* 
 * The topology used to simulate this attack contains 7 nodes as follows:
 * n0 -> arini (UE legitimate User)
 * n1 -> endro (4 UE attackers)
 * n2 -> enb (base station connected to yuri)
 * n3 -> bob (receiver)
     n0
        \ pp1 
         \
          \       
           \            |
            n2 ---- n3
          ////    pp1
         ////    
        ////
       //// pp1 
     a1a2a3a4
*/

#include "ns3/nstime.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/mmwave-helper.h"
#include "ns3/applications-module.h"


//Main function
int main (int argc, char *argv[])
{
  
  Simulator::Run();
  Simulator::Destroy();
  return 0;
}