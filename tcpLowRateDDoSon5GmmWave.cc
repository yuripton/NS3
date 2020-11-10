/* 
 * The topology used to simulate this attack contains 4 nodes as follows:
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
     n1
*/

#include "ns3/nstime.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"

