#include <iostream>
#include <ostream>
#include <span>
#include <array>
#include <unordered_set>
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
#include "include/frame-viewers/dns_viewer.h"

#include "include/frame-filter/frame_filter.h"

#include "include/utils/scoped_lock.h"
#include "include/utils/args_parser.h"
#include "include/utils/assert.h"

using ConstRawFrameViewType = posnet::EthernetViewer::ConstRawFrameViewType;
using RawFrameViewType = posnet::EthernetViewer::RawFrameViewType;

const auto ANY_PORTS = std::unordered_set<int>{};
const auto ANY_ADDRS = std::unordered_set<std::string>{};
const auto ANY_PROTOCOLS = std::unordered_set<std::string>{};

void Usage(const std::string_view appName)
{
    std::stringstream ss;
    ss << "Usage" << "\n";
    ss << "\t" << appName << " [options]" << "\n";
    ss << "Options:" << "\n";
    ss << "\t" << "--help                                                     = Print help info/usage" << "\n";
    ss << "\t" << "--port=num1, num2, ... | 0(any)                            = Filter a package with source and destination port number" << "\n";
    ss << "\t" << "--port-src=num1, num2, ... | 0(any)                        = Filter a package with source port number" << "\n";
    ss << "\t" << "--port-dst=num1, num2, ... | 0(any)                        = Filter a package with destination port number" << "\n";
    ss << "\t" << "--ip-addr=addr1, addr2, ... | any                          = Filter a package with source and destination ip address" << "\n";
    ss << "\t" << "--ip-src-addr=addr1, addr2, ... | any                      = Filter a package with source ip address" << "\n";
    ss << "\t" << "--ip-dst-addr=addr1, addr2, ... | any                      = Filter a package with source ip address" << "\n";
    ss << "\t" << "--eth-addr=addr1, addr2, ... | any                         = Filter a package with source and destination hardware address" << "\n";
    ss << "\t" << "--eth-src-addr=addr1, addr2, ... | any                     = Filter a package with source hardware address" << "\n";
    ss << "\t" << "--eth-dst-addr=addr1, addr2, ... | any                     = Filter a package with destination hardware address" << "\n";
    ss << "\t" << "--protocols=protocol1, protocol2, ... | any                = Filter a package with protocol" << "\n";
    ss << "\t" << "         supported protocols list=tcp, udp" << "\n";
    std::cout << ss.str() << std::endl;
}

void ParseCapturedFrameOnL3(
    const posnet::IpViewer& ipViewer, 
    const posnet::FrameFilter& filter, 
    std::stringstream& ss
) {
    switch (ipViewer.getProtocol()) {
        using ProtocolType = posnet::IpViewer::ProtocolType;
        case ProtocolType::TCP: {
            const posnet::TcpViewer tcpViewer{ ipViewer };
            if (!filter.filter(tcpViewer)) {
                ss << tcpViewer << std::endl;
            } else {
                ss.clear();
            }
            break;
        }
        case ProtocolType::UDP: {
            const posnet::UdpViewer udpViewer{ ipViewer };
            if (!filter.filter(udpViewer)) {
                ss << udpViewer << std::endl;
            } else {
                ss.clear();
            }
            break;
        }
        case ProtocolType::ICMP: {
            const posnet::IcmpViewer icmpViewer{ ipViewer };
            if (!filter.filter(icmpViewer)) {
                ss << icmpViewer << std::endl;
            } else {
                ss.clear();
            }
            break;
        }
        default: {
            ss.clear();
        }
    }
}

void ParseCapturedFrameOnL3(
    const posnet::ArpViewer& arpViewer, 
    const posnet::FrameFilter& filter, 
    std::stringstream& ss
) {
    if (!filter.filter(arpViewer)) {
        ss << arpViewer << std::endl;
    } else {
        ss.clear();
    }
}

void ParseCapturedFrameOnL2(
    const posnet::EthernetViewer& ethernetViewer, 
    const posnet::FrameFilter& filter, 
    std::stringstream& ss
) {
    switch (ethernetViewer.getProtocol()) {
        using ProtocolType = posnet::EthernetViewer::ProtocolType; 
        case ProtocolType::IP: {
            const posnet::IpViewer ipViewer{ ethernetViewer };
            if (!filter.filter(ipViewer)) {
                ss << ipViewer << std::endl;
                ParseCapturedFrameOnL3(ipViewer, filter, ss);
            } else {
                ss.clear();
            }
            break;
        }
        case ProtocolType::ARP: {
            const posnet::ArpViewer arpViewer{ ethernetViewer };
            if (!filter.filter(arpViewer)) {
                ss << arpViewer << std::endl;
                ParseCapturedFrameOnL3(arpViewer, filter, ss);
            } else {
                ss.clear();
            }
            break;
        }
        default: {
            ss.clear();
            break;
        }
    }
}

void ParseCapturedFrameOnL1(
    const ConstRawFrameViewType rawFrameBuffer, 
    const posnet::FrameFilter& filter, 
    std::stringstream& ss
) {
    const posnet::EthernetViewer ethernetViewer{ rawFrameBuffer };
    if (!filter.filter(ethernetViewer)) {
        ss << ethernetViewer << std::endl;
        ParseCapturedFrameOnL2(ethernetViewer, filter, ss);
    } else {
        ss.clear();
    }
}

void ParseCapturedFrame(
    const ConstRawFrameViewType rawFrameBuffer, 
    const posnet::FrameFilter& filters, 
    std::stringstream& ss
) {
    ParseCapturedFrameOnL1(rawFrameBuffer, filters, ss);
}

posnet::FrameFilter GetFilter(posnet::utils::args::Parser&& parser)
{
    posnet::FrameFilter filter;
    // Parse transport port
    {
        auto portData = parser.getArgValue<std::unordered_set<int>>("--port", ',').value_or(ANY_PORTS);
        filter.srcPort.insert(portData.begin(), portData.end());
        filter.dstPort.insert(portData.begin(), portData.end());

        portData = parser.getArgValue<std::unordered_set<int>>("--port-src", ',').value_or(ANY_PORTS);
        filter.srcPort.insert(portData.begin(), portData.end());

        portData = parser.getArgValue<std::unordered_set<int>>("--port-dst", ',').value_or(ANY_PORTS);
        filter.dstPort.insert(portData.begin(), portData.end());
    }

    // Parse ip addr
    {
        auto ipAddrData = parser.getArgValue<std::unordered_set<std::string>>("--ip-addr", ',').value_or(ANY_ADDRS);
        filter.srcIpAddr.insert(ipAddrData.begin(), ipAddrData.end());
        filter.dstIpAddr.insert(ipAddrData.begin(), ipAddrData.end());


        ipAddrData = parser.getArgValue<std::unordered_set<std::string>>("--ip-src-addr", ',').value_or(ANY_ADDRS);
        filter.srcIpAddr.insert(ipAddrData.begin(), ipAddrData.end());

        ipAddrData = parser.getArgValue<std::unordered_set<std::string>>("--ip-dst-addr", ',').value_or(ANY_ADDRS);
        filter.dstIpAddr.insert(ipAddrData.begin(), ipAddrData.end());
    }

    // Parse eth addr
    {
        auto ethAddrData = parser.getArgValue<std::unordered_set<std::string>>("--eth-addr", ',').value_or(ANY_ADDRS);
        filter.srcHrwAddr.insert(ethAddrData.begin(), ethAddrData.end());
        filter.dstHrwAddr.insert(ethAddrData.begin(), ethAddrData.end());

        ethAddrData = parser.getArgValue<std::unordered_set<std::string>>("--eth-src-addr", ',').value_or(ANY_ADDRS);
        filter.srcHrwAddr.insert(ethAddrData.begin(), ethAddrData.end());

        ethAddrData = parser.getArgValue<std::unordered_set<std::string>>("--eth-dst-addr", ',').value_or(ANY_ADDRS);
        filter.dstHrwAddr.insert(ethAddrData.begin(), ethAddrData.end());
    }

    filter.protocols = parser.getArgValue<std::unordered_set<std::string>>("--protocols", ',').value_or(ANY_PROTOCOLS);
    return filter;
}

int main(int argc, char** argv) {
    try {
        posnet::utils::args::Parser parser(argc, argv);
        (void)parser.addArgPattern("--help")
            .addArgPattern("--port")
            .addArgPattern("--port-src")
            .addArgPattern("--port-dst")
            .addArgPattern("--ip-addr")
            .addArgPattern("--ip-src-addr")
            .addArgPattern("--ip-dst-addr")
            .addArgPattern("--eth-addr")
            .addArgPattern("--eth-src-addr")
            .addArgPattern("--eth-dst-addr")
            .addArgPattern("--protocols")
            .parse();

        if (parser.hasArg("--help")) {
            Usage(argv[0]);
            return EXIT_SUCCESS;
        }

        {
            const auto ifConfig = posnet::GetFirstNonLoopbackIface();
            ASSERTION(ifConfig, std::runtime_error, "Could not find non loopback iface")
            ASSERTION(ifConfig->getName(), std::runtime_error, "Could not get non loopback iface name")

            posnet::IFaceManager ifManager;
            ifManager.enablePromiscuousMode(*ifConfig->getName());
        }

        const auto filter = GetFilter(std::move(parser));
        const auto sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if (sockfd < 0) {
            std::cerr << "Could not create socket for listening" << std::endl;
            return EXIT_FAILURE;
        }

        posnet::utils::ScopedLock lock([sockfd]{ close(sockfd); });
        std::array<posnet::def::ByteType, 65536> buffer = {0};
        struct sockaddr sockaddr;
        std::memset(&sockaddr, 0, sizeof(sockaddr));
        socklen_t sockaddrSize = sizeof(sockaddr);
        std::stringstream ss;

        while (true) {
            const std::size_t bufferSize = recvfrom(sockfd, buffer.data(), buffer.size(), 0, &sockaddr, &sockaddrSize);
            if (bufferSize < 0) {
                std::cerr << "Failed to get packets" << std::endl;
                return EXIT_FAILURE;
            }

            ParseCapturedFrame(ConstRawFrameViewType{ buffer.data(), bufferSize}, filter, ss);
            std::cout << ss.str();
            ss.clear();
        }

        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Throw unknown exception" << std::endl;
        return EXIT_FAILURE;
    }
}
