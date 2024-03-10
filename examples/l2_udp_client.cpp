#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/ether.h>
#include <netinet/if_ether.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <sys/ioctl.h>
#include <net/if.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#define UDP_DATA "Hello, UDP Server!"

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[IP_MAXPACKET];
    struct iphdr *ip_header;
    struct udphdr *udp_header;

    // Create a raw socket
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set up the server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345); // Replace with your server port
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr); // Replace with your server IP

    // Fill in the IP header
    ip_header = (struct iphdr *)buffer;
    ip_header->ihl = 5;
    ip_header->version = 4;
    ip_header->tos = 0;
    ip_header->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(UDP_DATA));
    ip_header->id = htons(12345); // You can choose a random ID
    ip_header->frag_off = 0;
    ip_header->ttl = 64;
    ip_header->protocol = IPPROTO_UDP;
    ip_header->check = 0; // We set it to 0 before calculating the actual checksum
    ip_header->saddr = inet_addr("127.0.0.1"); // Replace with your source IP
    ip_header->daddr = server_addr.sin_addr.s_addr;

    // Calculate the IP checksum
    ip_header->check = htons(~in_cksum((unsigned short *)buffer, sizeof(struct iphdr)));

    // Fill in the UDP header
    udp_header = (struct udphdr *)(buffer + sizeof(struct iphdr));
    udp_header->source = htons(12345); // Replace with your source port
    udp_header->dest = htons(12345); // Replace with your destination port
    udp_header->len = htons(sizeof(struct udphdr) + strlen(UDP_DATA));
    udp_header->check = 0; // We set it to 0 before calculating the actual checksum

    // Copy the UDP data
    memcpy(buffer + sizeof(struct iphdr) + sizeof(struct udphdr), UDP_DATA, strlen(UDP_DATA));

    // Calculate the UDP checksum
    udp_header->check = htons(~in_cksum((unsigned short *)(buffer + sizeof(struct iphdr)), sizeof(struct udphdr) + strlen(UDP_DATA)));

    // Send the packet
    if (sendto(sockfd, buffer, ip_header->tot_len, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("sendto");
        exit(EXIT_FAILURE);
    }

    close(sockfd);
    return 0;
}

// Function to calculate the checksum
unsigned short in_cksum(unsigned short *addr, int len) {
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1) {
        *(unsigned char *)(&answer) = *(unsigned char *)w;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return answer;
}



// #include <iostream>
// #include <string>
// #include <string_view>
// #include <array>
// #include <algorithm>

// #include <cstring>
// #include <cassert>

// #include "include/net-iface/iface_manager.h"

// #include "include/frame-builder/ethernet_builder.h"
// #include "include/frame-builder/ip_builder.h"
// #include "include/frame-builder/udp_builder.h"

// #include "include/utils/algorithms.h"
// #include "include/utils/scoped_lock.h"

// #include <linux/if_packet.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <net/ethernet.h> 
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <sys/socket.h>

// #define DEBUG

// int main() {
//     constexpr std::string_view PAYLOAD("Hello, UDP server!");
//     constexpr auto PORT = 12345;

//     std::array<posnet::def::ByteType, 1024> buffer = {0};
//     posnet::def::SizeType bufferSize = 0;
    
//     std::string myMacAddr;
//     std::string myIpAddr;

//     // Set up socket address structure
//     struct sockaddr_ll sockAddr;
//     {
//         posnet::IFaceManager ifaceManager;
//         const auto configs = ifaceManager.getConfigs();
//         const auto it = std::find_if(configs.cbegin(), configs.cend(), [](const posnet::IFaceConfiguration& config) {
//             return (config.getName() && (*config.getName() != posnet::IFaceConfiguration::LOOP_BACK_INTERFACE_NAME));
//         });

//         if (it == configs.cend()) {
//             return EXIT_FAILURE;
//         }

//         assert(it->getIndex());
//         assert(it->getMacAddress());
//         assert(it->getIpAddress());

//         std::memset(&sockAddr, 0, sizeof(sockAddr));
//         sockAddr.sll_family = AF_PACKET;
//         sockAddr.sll_protocol = htons(ETH_P_IP);
//         sockAddr.sll_ifindex = *it->getIndex();

//         myMacAddr = *it->getMacAddress();
//         myIpAddr = *it->getIpAddress();
//     }

//     // Building frame
//     {
//         posnet::EthernetBuilder ethernetBuilder;
//         ethernetBuilder.setProtocol(posnet::EthernetBuilder::ProtocolType::IP)
//                 .setDestMacAddress(myMacAddr)
//                 .setSourceMacAddress(myMacAddr);

//         posnet::IpBuilder ipBuilder;
//         ipBuilder.setHeaderLengthInBytes(posnet::IpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES)
//                 .setVersion(posnet::IpBuilder::VersionType::V4)
//                 .setTypeOfService(0)
//                 .setId(0)
//                 .setTTL(64)
//                 .setProtocol(posnet::IpBuilder::ProtocolType::UDP)
//                 .setSourceIpAddress(myIpAddr)
//                 .setDestIpAddress(myIpAddr);

//         posnet::UdpBuilder udpBuilder;
//         udpBuilder.setSourcePort(PORT + 1)
//                 .setDestPort(PORT)
//                 .setCheckSum(0)
//                 .setUdpDataGramLength(posnet::UdpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + PAYLOAD.size());

//         ipBuilder.setTotalLength(posnet::IpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + 
//                                     posnet::UdpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + PAYLOAD.size());
//         ipBuilder.setCheckSum(posnet::utils::CalcChecksum(ipBuilder.getAsRawFrameView()));


//         // filling the buffer
//         std::memcpy(buffer.data() + bufferSize, ethernetBuilder.getStart(), ethernetBuilder.getSize());
//         bufferSize += ethernetBuilder.getSize();

//         std::memcpy(buffer.data() + bufferSize, ipBuilder.getStart(), ipBuilder.getSize());
//         bufferSize += ipBuilder.getSize();

//         std::memcpy(buffer.data() + bufferSize, udpBuilder.getStart(), udpBuilder.getSize());
//         bufferSize += udpBuilder.getSize();

//         std::memcpy(buffer.data() + bufferSize, PAYLOAD.data(), PAYLOAD.size());
//         bufferSize += PAYLOAD.size();

// #ifdef DEBUG
//         std::cout << ethernetBuilder << std::endl;
//         std::cout << ipBuilder << std::endl;
//         std::cout << udpBuilder << std::endl;
// #endif //! DEBUG
//     }

//      // Create a raw socket for sending
//     int sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
//     posnet::utils::ScopedLock socketLock([sock]{
//         (void)close(sock);
//     });

//     if (sock == -1) {
//         perror("socket");
//         return EXIT_FAILURE;
//     }
    
//     // Send the packet
//     if (sendto(sock, buffer.data(), bufferSize, 0, (struct sockaddr*)&sockAddr, sizeof(sockAddr)) == -1) {
//         perror("sendto");
//         return EXIT_FAILURE;
//     }

//     std::cout << "Sent the Udp request successfully" << std::endl;
//     return EXIT_SUCCESS;
// }
