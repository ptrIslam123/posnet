#include <iostream>
#include <algorithm>
#include <span>
#include <array>
#include <iomanip>

#include "core/net-iface/iface_manager.h"
#include "core/frame-viewers/ethernet_viewer.h"
#include "core/frame-viewers/ip_viewer.h"
#include "core/frame-viewers/udp_viewer.h"
#include "core/frame-viewers/tcp_viewer.h"
#include "core/frame-viewers/icmp_viewer.h"
#include "core/frame-viewers/arp_viewer.h"

#include <cstring>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>

using namespace net::posix;

int main() {
    low_level::IFaceManager iface;
    auto configs = iface.getConfigs();
    auto it = std::find_if(configs.cbegin(), configs.cend(), [](const low_level::IFaceManager::Configuration& config) {
        return (config.getName() && *config.getName() != "lo");
    });

    if (it == configs.cend()) {
        return 0;
    }

    auto ifaceConf = *it;
    auto sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd == -1) {
        perror("socket");
        return 0;
    }

    struct sockaddr_ll sll;
    memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_protocol = htons(ETH_P_ALL);
    sll.sll_ifindex = if_nametoindex(ifaceConf.getName()->data()); // Replace with your interface name
    if (bind(sockfd, (struct sockaddr*)&sll, sizeof(sll)) == -1) {
        perror("bind");
        close(sockfd);
        return 0;
    }

    std::array<unsigned char, 65536> buffer = {};
    while (true) {
        ssize_t packet_size = recvfrom(sockfd, buffer.data(), buffer.size(), 0, NULL, NULL);
        if (packet_size == -1) {
            perror("recvfrom");
            break;
        }

        low_level::EthernetViewer ethernetViewer(buffer);
        if (ethernetViewer.getProtocol() == low_level::EthernetViewer::ProtocolType::IP) {
            //std::cout << ethernetViewer << std::endl;
            low_level::IpViewer ipViewer(ethernetViewer);
            switch (ipViewer.getProtocol()) {
                case low_level::IpViewer::ProtocolType::TCP: {
                    // std::cout << ipViewer << std::endl;
                    // low_level::TcpViewer tcpViewer(ipViewer);
                    // std::cout << tcpViewer << std::endl;
                    break;
                } 
                case low_level::IpViewer::ProtocolType::UDP: {
                    // std::cout << ipViewer << std::endl;
                    // low_level::UdpViewer updViewer(ipViewer);
                    // std::cout << updViewer << std::endl;
                    break;
                }
                case low_level::IpViewer::ProtocolType::ICMP: {
                    low_level::IcmpViewer icmpViewer(ipViewer);
                    std::cout << icmpViewer << std::endl;
                    break;
                }
            }
        } else if (ethernetViewer.getProtocol() == low_level::EthernetViewer::ProtocolType::ARP) {
            low_level::ArpViewer arpViewer(ethernetViewer);
            std::cout << arpViewer << std::endl;
        }
    }
    return 0;
}
