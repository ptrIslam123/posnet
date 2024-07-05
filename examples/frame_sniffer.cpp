#include <iostream>
#include <ostream>
#include <fstream>
#include <filesystem>
#include <span>
#include <array>
#include <unordered_set>
#include <optional>
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <cassert>

#include "include/frame-filter/frame_filter.h"
#include "include/frame-capturer/frame_capturer.h"
#include "include/net-iface/iface_manager.h"

#include "include/frame-viewers/ethernet_viewer.h"
#include "include/frame-viewers/ip_viewer.h"
#include "include/frame-viewers/udp_viewer.h"
#include "include/frame-viewers/tcp_viewer.h"
#include "include/frame-viewers/icmp_viewer.h"
#include "include/frame-viewers/arp_viewer.h"

#include "include/utils/scoped_lock.h"
#include "include/utils/args_parser.h"
#include "include/utils/assert.h"

using ConstRawFrameViewType = posnet::EthernetViewer::ConstRawFrameViewType;
using RawFrameViewType = posnet::EthernetViewer::RawFrameViewType;

void Usage(const std::string_view appName)
{
    std::stringstream ss;
    ss << "Usage" << "\n";
    ss << "\t" << appName << " [options]" << "\n";
    ss << "Options:" << "\n";
    ss << "\t" << "--help                                                     = Print help info/usage" << "\n";
    ss << "\t" << "--debug                                                    = Print debug information" << "\n";
    ss << "\t" << "--out-file=file-path                                       = Set up a file for output information" << "\n";
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
    ss << "\t" << "         supported protocols list=| ";
    for (const auto proto : posnet::FrameFilter::GetSupportedProtocols()) {
        ss << proto << " | ";
    }
    ss << std::endl;
    std::cout << ss.str() << std::endl;
}

posnet::FrameFilter GetFilter(posnet::utils::args::Parser&& parser)
{
    posnet::FrameFilter filter;
    // Parse transport port
    {
        auto portData = parser.getArgValue<std::unordered_set<int>>("--port", ',');
        if (portData.has_value()) {
            filter.srcPort.insert(portData->begin(), portData->end());
            filter.dstPort.insert(portData->begin(), portData->end());
        }

        portData = parser.getArgValue<std::unordered_set<int>>("--port-src", ',');
        if (portData.has_value()) {
            filter.srcPort.insert(portData->begin(), portData->end());
        }


        portData = parser.getArgValue<std::unordered_set<int>>("--port-dst", ',');
        if (portData.has_value()) {
            filter.dstPort.insert(portData->begin(), portData->end());
        }

        if (filter.srcPort.empty()) {
            filter.srcPort.insert(posnet::FrameFilter::ANY_PORT);
        }

        if (filter.dstPort.empty()) {
            filter.dstPort.insert(posnet::FrameFilter::ANY_PORT);
        }
    }

    // Parse ip addr
    {
        auto ipAddrData = parser.getArgValue<std::unordered_set<std::string>>("--ip-addr", ',');
        if (ipAddrData.has_value()) {
            filter.srcIpAddr.insert(ipAddrData->begin(), ipAddrData->end());
            filter.dstIpAddr.insert(ipAddrData->begin(), ipAddrData->end());
        }

        ipAddrData = parser.getArgValue<std::unordered_set<std::string>>("--ip-src-addr", ',');
        if (ipAddrData.has_value()) {
            filter.srcIpAddr.insert(ipAddrData->begin(), ipAddrData->end());
        }

        ipAddrData = parser.getArgValue<std::unordered_set<std::string>>("--ip-dst-addr", ',');
        if (ipAddrData.has_value()) {
            filter.srcIpAddr.insert(ipAddrData->begin(), ipAddrData->end());
        }

        if (filter.srcIpAddr.empty()) {
            filter.srcIpAddr.insert(std::string{ posnet::FrameFilter::ANY_IP_ADDR });
        }

        if (filter.dstIpAddr.empty()) {
            filter.dstIpAddr.insert(std::string{ posnet::FrameFilter::ANY_IP_ADDR });
        }
    }

    // Parse eth addr
    {
        auto ethAddrData = parser.getArgValue<std::unordered_set<std::string>>("--eth-addr", ',');
        if (ethAddrData.has_value()) {
            filter.srcHrwAddr.insert(ethAddrData->begin(), ethAddrData->end());
            filter.dstHrwAddr.insert(ethAddrData->begin(), ethAddrData->end());
        }

        ethAddrData = parser.getArgValue<std::unordered_set<std::string>>("--eth-src-addr", ',');
        if (ethAddrData.has_value()) {
            filter.srcHrwAddr.insert(ethAddrData->begin(), ethAddrData->end());
        }
        

        ethAddrData = parser.getArgValue<std::unordered_set<std::string>>("--eth-dst-addr", ',');
        if (ethAddrData.has_value()) {
            filter.dstHrwAddr.insert(ethAddrData->begin(), ethAddrData->end());
        }

        if (filter.srcHrwAddr.empty()) {
            filter.srcHrwAddr.insert(std::string{ posnet::FrameFilter::ANY_ETH_ADDR });
        }

        if (filter.dstHrwAddr.empty()) {
            filter.dstHrwAddr.insert(std::string{ posnet::FrameFilter::ANY_ETH_ADDR });
        }
    }

    // Parse protocols
    {
        auto protocols = parser.getArgValue<std::unordered_set<std::string>>("--protocols", ',');
        if (protocols.has_value()) {
            filter.protocols = *protocols;
        } else {
            const auto anyProtocols = posnet::FrameFilter::GetSupportedProtocols();
            for (const auto& protocol : anyProtocols) {
                filter.protocols.insert(std::string{ protocol });
            }
        }

        if (filter.protocols.find("tcp") != filter.protocols.cend() ||
                filter.protocols.find("udp") != filter.protocols.cend() ||
                filter.protocols.find("icmp") != filter.protocols.cend()) {
            for (const auto& protocol : {"eth", "ip"}) {
                filter.protocols.insert(protocol);
            }
        }

        if (filter.protocols.find("arp") != filter.protocols.cend() ||
                filter.protocols.find("rarp") != filter.protocols.cend()) {
            for (const auto& protocol : {"eth"}) {
                filter.protocols.insert(protocol);
            }
        }
    }
    return filter;
}

void OnCapturedFrame(
    std::ostream& os, 
    const posnet::BaseFrame& baseFrame, 
    const posnet::FrameFilter::ProtocolType protocol,
    const int level
) {
    using ProtocolType = posnet::FrameFilter::ProtocolType;
    posnet::utils::ScopedLock lock([&os]{
        os.flush();
    });
    const auto rawFrame = baseFrame.getAsRawFrameView();

    if (level >= 1) {
        os << posnet::EthernetViewer{ rawFrame } << std::endl;
        if (level == 1) {
            return;
        }
    }

    // Already have printed ethernet level
    if (level >= 2) {
        if (protocol == ProtocolType::ARP || protocol == ProtocolType::RARP) {
            os << posnet::ArpViewer{ rawFrame } << std::endl;
        } else {
            switch (protocol) {
                case ProtocolType::IP:
                case ProtocolType::TCP:
                case ProtocolType::UDP:
                case ProtocolType::ICMP: {
                    os << posnet::IpViewer{ rawFrame } << std::endl;
                    break;
                }
                default: {
                    //! Do nothing
                    break;
                }
            }   
        }

        if (level == 2) {
            return;
        }
    }

    // Already have printed ip/arp level
    if (level >= 3) {
        const auto printPayload = [&os](const auto& payload) {
            os << "Payload(ASCI) {" << "\n"; 
            os << std::string_view{ (char*)(payload.data()), payload.size() };
            os << "\n}\n";

            os << "Payload(HEX) {" << "\n";
            //posnet::utils::DumpToHexFormat(os, payload);
            os << "\n}\n";
        };

        switch (protocol) {
            case ProtocolType::TCP: {
                const posnet::TcpViewer tcp{ rawFrame };
                os << tcp << std::endl;
                printPayload(tcp.getPayload());
                break;
            }
            case ProtocolType::UDP: {
                const posnet::UdpViewer udp{ rawFrame };
                os << udp << std::endl;
                printPayload(udp.getPayload());
                break;
            }
            case ProtocolType::ICMP: {
                os << posnet::IcmpViewer{ rawFrame } << std::endl;
                break;
            }
            default: {
                //! Do nothing
                break;
            }
        }
    }

    // Already have printed tcp/udp/icmp level
    if (level == 4) {
        //! TODO:
    }
}

int main(int argc, char** argv) {
    try {
        posnet::utils::args::Parser parser(argc, argv);
        (void)parser.addArgPattern("--help")
            .addArgPattern("--debug")
            .addArgPattern("--out-file")
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

        std::ostream* outStream = nullptr;
        if (parser.hasArg("--out-file")) {
            const auto outFilePath = parser.getArgValue<std::string>("--out-file", '=').value_or(".");
            static std::ofstream fileStream(outFilePath.c_str());
            ASSERTION(fileStream.is_open(), std::runtime_error, "Could not open out file by path" + outFilePath);
            outStream = &fileStream;
        } else {
            outStream = &std::cout;
        }

        {
            const auto ifConfig = posnet::GetFirstNonLoopbackIface();
            ASSERTION(ifConfig, std::runtime_error, "Could not find non loopback iface")
            ASSERTION(ifConfig->getName(), std::runtime_error, "Could not get non loopback iface name")

            posnet::IFaceManager ifManager;
            ifManager.enablePromiscuousMode(*ifConfig->getName());
        }

        const auto filter = GetFilter(std::move(parser));
        if (parser.hasArg("--debug")) {
            std::cout << filter << std::endl;
        }

        posnet::FrameCapturer capturer;
        capturer.capture(filter, [&os = *outStream](
            const posnet::BaseFrame& baseFrame, 
            const posnet::FrameFilter::ProtocolType protocol,
            const int level
            ) {
            OnCapturedFrame(os, baseFrame, protocol, level);
            return posnet::FrameCapturer::CONTINUE_CAPTURING;
        });
        
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Throw unknown exception" << std::endl;
        return EXIT_FAILURE;
    }
}
