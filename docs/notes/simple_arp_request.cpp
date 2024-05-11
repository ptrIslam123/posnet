#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <netinet/if_ether.h>
#include <cassert>
#include "include/utils/sock_addr_convertor.h"

#define SRC_IP_ADDR "10.110.15.84"
#define SRC_MAC_ADDR "58:11:22:06:96:bc"
#define DST_IP_ADDR "10.110.15.10"
#define DST_MAC_ADDR "50:eb:f6:9e:db:c8"

#define ETH_ALEN 6
#define INLEN 4
#define MAC_BCAST_ADDR  "\xff\xff\xff\xff\xff\xff"
#define IF_INTERFACE "eno1"

// tcpdump arp host 10.110.15.10
int main(int argc, char **argv)
{
    int reqfd;
    socklen_t salen;
    struct sockaddr_ll reqsa;
    struct arp_pkt {
        struct ether_header eh;
        struct ether_arp ea;
        u_char padding[18];
    } req;

    bzero(&reqsa, sizeof(reqsa));
    reqsa.sll_family = PF_PACKET;
    reqsa.sll_ifindex = if_nametoindex(IF_INTERFACE);

    if((reqfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP))) < 0) {
        perror("Socket error");
        exit(1);
    }

    /* Заполнение заголовка Ethernet */
    memcpy(req.eh.ether_dhost, MAC_BCAST_ADDR, ETH_ALEN);
    memset(req.eh.ether_shost, 0x00, ETH_ALEN); // Заполните это значением вашего MAC-адреса
    req.eh.ether_type = htons(ETHERTYPE_ARP);

    /* Заполнение данных ARP */
    req.ea.arp_hrd = htons(ARPHRD_ETHER);
    req.ea.arp_pro = htons(ETHERTYPE_IP);
    req.ea.arp_hln = ETH_ALEN;
    req.ea.arp_pln = INLEN;
    req.ea.arp_op = htons(ARPOP_REQUEST);
    memset(req.ea.arp_sha, 0x00, ETH_ALEN); // Заполните это значением вашего MAC-адреса
    {
        auto addr = posnet::utils::StrToMacAddr(SRC_MAC_ADDR);
        assert(addr);
        memcpy(req.ea.arp_sha, addr->data(), addr->size());
    }
    memset(req.ea.arp_spa, 0x00, INLEN); // Заполните это значением вашего IP-адреса
    {
        auto addr = posnet::utils::StrToIpAddrArray(SRC_IP_ADDR);
        memcpy(req.ea.arp_spa, addr->data(), addr->size());
    }
    memset(req.ea.arp_tha, 0x00, ETH_ALEN); // Заполните это значением MAC-адреса назначения
    memset(req.ea.arp_tpa, 0x00, INLEN); // Заполните это значением IP-адреса назначения
    {
        auto addr = posnet::utils::StrToIpAddrArray(DST_IP_ADDR);
        memcpy(req.ea.arp_tpa, addr->data(), addr->size());
    }

    if(sendto(reqfd, &req, sizeof(req), 0, (struct sockaddr *)&reqsa, sizeof(reqsa)) <= 0) {
        perror("Sendto error");
        exit(1);
    }

    printf("Broadcast ARP request sent\n");

    close(reqfd);
    return 0;
}
