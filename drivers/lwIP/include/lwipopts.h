//
// Created by sacha on 10/29/26.
//

#pragma once

#include "arch/cc.h"

#define NO_SYS 1
#define LWIP_SOCKET 0
#define LWIP_NETCONN 0
#define LWIP_TCP 1
#define LWIP_ARP 1
#define LWIP_ICMP 1
#define LWIP_UDP 1
#define LWIP_DHCP 1

#define MEM_SIZE (32 * 1024)
#define PBUF_POOL_SIZE 32
#define PBUF_POOL_BUFSIZE 1536