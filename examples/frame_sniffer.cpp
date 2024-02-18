#include <iostream>
#include <ostream>
#include <fstream>
#include <span>
#include <array>
#include <optional>
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <cassert>

#include <sys/socket.h>
#include <unistd.h>

#include "include/net-iface/iface_manager.h"
#include "include/utils/system_error.h"

#include "include/frame-viewers/ethernet_viewer.h"
#include "include/frame-viewers/ip_viewer.h"
#include "include/frame-viewers/udp_viewer.h"
#include "include/frame-viewers/tcp_viewer.h"
#include "include/frame-viewers/icmp_viewer.h"
#include "include/frame-viewers/arp_viewer.h"

using ConstRawFrameViewType = posnet::EthernetViewer::ConstRawFrameViewType;
using RawFrameViewType = posnet::EthernetViewer::RawFrameViewType;

void PrintTcpFrameInfo(const ConstRawFrameViewType rawFrameBuffer, std::ostream& os) {
    posnet::TcpViewer tcpViewer(rawFrameBuffer);
    os << tcpViewer << "\n";
}

void PrintUdpFrameInfo(const ConstRawFrameViewType rawFrameBuffer, std::ostream& os) {
    posnet::UdpViewer udpViewer(rawFrameBuffer);
    os << udpViewer << "\n";
}

void PrintIcmpFrameInfo(const ConstRawFrameViewType rawFrameBuffer, std::ostream& os) {
    posnet::IcmpViewer icmpViewer(rawFrameBuffer);
    os << icmpViewer << "\n";
}

void PrintArpFrameInfo(const ConstRawFrameViewType rawFrameBuffer, std::ostream& os) {
    posnet::ArpViewer arpViewer(rawFrameBuffer);
    os << arpViewer << "\n";
}

void PrintRArpFrameInfo(const ConstRawFrameViewType rawFrameBuffer, std::ostream& os) {
    posnet::ArpViewer arpViewer(rawFrameBuffer);
    os << "(!RARP frame)" << arpViewer << "\n";
}

void PrintIpFrameInfo(const ConstRawFrameViewType rawFrameBuffer, std::ostream& os) {
    posnet::IpViewer ipViewer(rawFrameBuffer);
    os << ipViewer << "\n";
    switch (ipViewer.getProtocol()) {
        using ProtocolType = posnet::IpViewer::ProtocolType;
        case ProtocolType::TCP: {
            PrintTcpFrameInfo(rawFrameBuffer, os);
            break;
        }
        case ProtocolType::UDP: {
            PrintUdpFrameInfo(rawFrameBuffer, os);
            break;
        }
        case ProtocolType::ICMP: {
            PrintIcmpFrameInfo(rawFrameBuffer, os);
            break;
        }
        default: {
            os << "Unknown frame" << "\n";
            break;
        }
    }
}


void PrintFrameInfo(const ConstRawFrameViewType rawFrameBuffer, std::ostream& os) {
    os << "----------------------------- RECEIVED A NEW FRAME HEADER START -----------------------------" << "\n";
    posnet::EthernetViewer ethernetViewer(rawFrameBuffer);
    os << ethernetViewer << "\n";
    switch (ethernetViewer.getProtocol()) {
        using ProtocolType = posnet::EthernetViewer::ProtocolType;
        case ProtocolType::ARP: {
            PrintArpFrameInfo(rawFrameBuffer, os);
            break;
        }
        case ProtocolType::IP: {
            PrintIpFrameInfo(rawFrameBuffer, os);
            break;
        }
        case ProtocolType::RARP: {
            PrintRArpFrameInfo(rawFrameBuffer, os);
            break;
        }
        default: {
            os << "Unknown frame" << "\n";
            break;
        }
    }
    os << "-----------------------------  RECEIVED A NEW FRAME HEADER END -----------------------------" << "\n\n";
}

int ParseFrames()
{
    const auto sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        perror("Socket Error");
        return EXIT_FAILURE;
    }

    std::array<std::uint8_t, 65536> buffer = {0};
    struct sockaddr sockaddr;
    socklen_t sockaddrSize = sizeof(sockaddr);
    std::memset(&sockaddr, 0, sockaddrSize);
    
    while (true) {
        const std::size_t bufferSize = recvfrom(sockfd, buffer.data(), buffer.size(), 0, &sockaddr, &sockaddrSize);
        if (bufferSize < 0) {
            std::cerr << "Failed to get packets" << std::endl;
            return EXIT_FAILURE;
        }

        PrintFrameInfo(ConstRawFrameViewType{ buffer.data(), bufferSize}, std::cout);
    }
}

std::optional<std::string> GetDefaultIFaceName(const posnet::IFaceManager& ifaceManager)
{
    const auto configs = ifaceManager.getConfigs();
    const auto it = std::find_if(configs.cbegin(), configs.cend(), [](const posnet::IFaceManager::Configuration& config) {
        return (config.getName() && *config.getName() != posnet::IFaceManager::LOOP_BACK_INTERFACE_NAME); 
    });

    return it != configs.cend() ? std::make_optional<std::string>(*(it->getName())) : std::nullopt;
}

void PrintHelpInfo()
{
    
}

int main(int argc, char** argv) {

    try {
        posnet::IFaceManager ifaceManager;
        const auto defaultIfaceName(GetDefaultIFaceName(ifaceManager));
        assert(defaultIfaceName);
        ifaceManager.enablePromiscuousMode(*defaultIfaceName);

        return ParseFrames();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Throw unknown exception" << std::endl;
        return EXIT_FAILURE;
    }
}