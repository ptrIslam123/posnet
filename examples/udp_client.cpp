#include <iostream>
#include <algorithm>
#include <span>
#include <string>
#include <string_view>
#include <array>
#include <iomanip>
#include <cassert>

#include "utils/algorithms.h"
#include "net-iface/iface_manager.h"

#include "frame-viewers/ethernet_viewer.h"
#include "frame-viewers/ip_viewer.h"
#include "frame-viewers/udp_viewer.h"
#include "frame-viewers/tcp_viewer.h"
#include "frame-viewers/icmp_viewer.h"
#include "frame-viewers/arp_viewer.h"

#include "frame-builder/ethernet_builder.h"
#include "frame-builder/ip_builder.h"
#include "frame-builder/udp_builder.h"

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

int frames_parser()
{
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

#include <iostream>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <type_traits>
#include <cstdint>

void ConvertStringToNetworkByteOrder(std::string& str) {
    // Проверка размера строки на кратность 2 байтам для 16-битных блоков
    if (str.size() % 2 != 0) {
        // Строка должна быть кратна 2 байтам для корректной обработки 16-битных блоков
        return;
    }

    // Преобразование каждого 16-битного блока строки в сетевой порядок байт
    for (size_t i = 0; i < str.size(); i += 2) {
        std::uint16_t* value = reinterpret_cast<std::uint16_t*>(&str[i]);
        *value = htons(*value);
    }
}

int main(int argc, char** argv) {
    constexpr auto PORT = 12345;
    int sockfd;
    struct sockaddr_ll sock_addr;
    std::array<u_char, 1024> buffer = {0};
    std::string ifaceName;
    std::string ifaceMacAddress;
    std::string message("Hello world!");

    ConvertStringToNetworkByteOrder(message);

    // Create a raw socket
    sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    // Detecting appropriate iface instance
    {
        low_level::IFaceManager ifaceMng;
        const auto configs = ifaceMng.getConfigs();
        auto it = std::find_if(configs.cbegin(), configs.cend(), [](const low_level::IFaceManager::Configuration& config) {
            return (config.getName() && *config.getName() != "lo");
        });

        if (it != configs.cend()) {
            assert(it->getName() && it->getMacAddress());
            ifaceName = *(it->getName());
            ifaceMacAddress = *(it->getMacAddress());

            std::cout << "Detected and chosen " << *it << std::endl; 
        } else {
            std::cerr << "Could not detect iface" << std::endl;
            return 1;
        }
    }

    // Set up frames header
    {
        low_level::EthernetBuilder ethernetBuilder(buffer);
        ethernetBuilder.setDestMacAddress(ifaceMacAddress)
            .setSourceMacAddress(ifaceMacAddress)
            .setProtocol(low_level::EthernetBuilder::ProtocolType::IP);

        low_level::IpBuilder ipBuilder(buffer);
        ipBuilder.setDestIpAddress(low_level::IpBuilder::LOCAL_HOST_ID_ADDRESS)
            .setSourceIpAddress(low_level::IpBuilder::LOCAL_HOST_ID_ADDRESS)
            .setCheckSum(ipBuilder.getDefaultCheckSum())
            .setFragmentOffset(low_level::IpBuilder::DEFAULT_FRAME_FRAGMENT_FLAG_VALUE, low_level::IpBuilder::DEFAULT_FRAME_FRAGMENT_OFFSET_VALUE)
            .setHeaderLengthInBytes(low_level::IpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES)
            .setId(low_level::IpBuilder::DEFAULT_FRAME_ID_VALUE)
            .setProtocol(low_level::IpBuilder::ProtocolType::UDP)
            .setVersion(low_level::IpBuilder::VersionType::V4)
            .setTotalLength(low_level::IpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + low_level::UdpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + message.size()/*USER PAYLOAD*/)
            .setTTL(low_level::IpBuilder::DEFAULT_FRAME_TTL_VALUE)
            .setTypeOfService(low_level::IpBuilder::DEFAULT_FRAME_TOS_VALUE);

        low_level::UdpBuilder udpBuilder(buffer);
        udpBuilder.setDestPort(PORT)
            .setSourcePort(PORT)
            .setUdpDataGramLength(0)
            .setCheckSum(udpBuilder.getDefaultCheckSum())
            .setPayload(std::span<std::uint8_t>(reinterpret_cast<std::uint8_t*>(message.data()), message.size()));

        std::cout << "------------------------------------------- Created tha UDP package successful -------------------------------------------" << std::endl;
        const low_level::EthernetViewer ethernetViewer(buffer);
        const low_level::IpViewer ipViewer(ethernetViewer);
        const low_level::UdpViewer udpViewer(ipViewer);
        std::cout << ethernetViewer << std::endl;
        std::cout << ipViewer << std::endl;
        std::cout << udpViewer << std::endl;
        std::cout << "---------------------------------------------------------------------------------------------------------------------------" << std::endl;
    }
    
    // Set up the socket address
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sll_family = AF_PACKET;
    sock_addr.sll_protocol = htons(ETH_P_ALL);
    sock_addr.sll_ifindex = if_nametoindex(ifaceName.c_str());
    sock_addr.sll_hatype = ARPHRD_ETHER;

    // Send the Ethernet frame
    const auto bufferSize = low_level::EthernetBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + 
        low_level::IpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + 
        low_level::UdpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + message.size();
    if (sendto(sockfd, buffer.data(), bufferSize, 0, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) < 0) {
        perror("sendto");
        close(sockfd);
        return 1;
    } else {
        std::cout << "Sent the raw UDP package to destination host successful" << std::endl;
    }

    // Close the socket
    return close(sockfd);
}