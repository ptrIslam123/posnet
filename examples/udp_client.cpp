#include <iostream>
#include <algorithm>
#include <span>
#include <string>
#include <string_view>
#include <array>
#include <iomanip>
#include <cassert>

#include "include/utils/algorithms.h"
#include "include/net-iface/iface_manager.h"

#include "include/frame-viewers/ethernet_viewer.h"
#include "include/frame-viewers/ip_viewer.h"
#include "include/frame-viewers/udp_viewer.h"
#include "include/frame-viewers/tcp_viewer.h"
#include "include/frame-viewers/icmp_viewer.h"
#include "include/frame-viewers/arp_viewer.h"

#include "include/frame-builder/ethernet_builder.h"
#include "include/frame-builder/ip_builder.h"
#include "include/frame-builder/udp_builder.h"

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
        posnet::IFaceManager ifaceMng;
        const auto configs = ifaceMng.getConfigs();
        auto it = std::find_if(configs.cbegin(), configs.cend(), [](const posnet::IFaceManager::Configuration& config) {
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
        posnet::EthernetBuilder ethernetBuilder(buffer);
        ethernetBuilder.setDestMacAddress(ifaceMacAddress)
            .setSourceMacAddress(ifaceMacAddress)
            .setProtocol(posnet::EthernetBuilder::ProtocolType::IP);

        posnet::IpBuilder ipBuilder(buffer);
        ipBuilder.setDestIpAddress(posnet::IpBuilder::LOCAL_HOST_ID_ADDRESS)
            .setSourceIpAddress(posnet::IpBuilder::LOCAL_HOST_ID_ADDRESS)
            .setCheckSum(ipBuilder.getDefaultCheckSum())
            .setFragmentOffset(posnet::IpBuilder::DEFAULT_FRAME_FRAGMENT_FLAG_VALUE, posnet::IpBuilder::DEFAULT_FRAME_FRAGMENT_OFFSET_VALUE)
            .setHeaderLengthInBytes(posnet::IpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES)
            .setId(posnet::IpBuilder::DEFAULT_FRAME_ID_VALUE)
            .setProtocol(posnet::IpBuilder::ProtocolType::UDP)
            .setVersion(posnet::IpBuilder::VersionType::V4)
            .setTotalLength(posnet::IpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + posnet::UdpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + message.size()/*USER PAYLOAD*/)
            .setTTL(posnet::IpBuilder::DEFAULT_FRAME_TTL_VALUE)
            .setTypeOfService(posnet::IpBuilder::DEFAULT_FRAME_TOS_VALUE);

        posnet::UdpBuilder udpBuilder(buffer);
        udpBuilder.setDestPort(PORT)
            .setSourcePort(PORT)
            .setUdpDataGramLength(0)
            .setCheckSum(udpBuilder.getDefaultCheckSum())
            .setPayload(std::span<std::uint8_t>(reinterpret_cast<std::uint8_t*>(message.data()), message.size()));

        std::cout << "------------------------------------------- Created tha UDP package successful -------------------------------------------" << std::endl;
        const posnet::EthernetViewer ethernetViewer(buffer);
        const posnet::IpViewer ipViewer(ethernetViewer);
        const posnet::UdpViewer udpViewer(ipViewer);
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
    const auto bufferSize = posnet::EthernetBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + 
        posnet::IpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + 
        posnet::UdpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + message.size();
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