#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <algorithm>
#include <optional>
#include <set>
#include <array>
#include <thread>
#include <mutex>
#include <cstring>
#include <cstdlib>
#include <cassert>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

#include "include/net-iface/iface_manager.h"

#include "include/utils/algorithms.h"
#include "include/utils/sock_addr_convertor.h"
#include "include/utils/scoped_lock.h"
#include "include/definitions.h"

#include "include/frame-viewers/ethernet_viewer.h"
#include "include/frame-viewers/ip_viewer.h"
#include "include/frame-viewers/icmp_viewer.h"

#include "include/frame-builder/icmp_builder.h"

constexpr std::string_view HELP_PARAM("--help");
constexpr std::string_view VERBOSE_LEVEL_PARAM("--verbose=");
constexpr std::string_view SCAN_IP_RANGE_PARAM("--range=");
constexpr std::string_view WAIT_TIMEOUT_PARAM("--timeout=");
constexpr std::string_view PING_PARAM("--ping=");

std::mutex gIOLock;

void LOG_ERROR(const std::string&& msg)
{
    std::unique_lock lock(gIOLock);
    std::cerr << "ERROR: " << msg << std::endl;
}

void LOG(const std::string&& msg)
{
    std::unique_lock lock(gIOLock);
    std::cerr << msg << std::endl;
}

void Usage()
{
    std::stringstream ss;
    ss << "Usage" << "\n";
    ss << "\t\ticmp-scanner [options]" << "\n";
    ss << "Options:" << "\n";
    ss << "\t--help                                         = Print help info/usage" << "\n";
    ss << "\t--verbose=<level>                              = Set up verbose level{0, 1}" << "\n";
    ss << "\t--range=[<start-ip-addr>-<end-ip-addr>]        = Set up ip address range for scanning" << "\n";
    ss << "\t--timeout=<value>                              = Waiting timeout of the icmp reply" << "\n";
    ss << "\t--ping=<ip-addr>                               = Just ping one host" << "\n";
    ss << std::endl;
    LOG(ss.str());
}

bool SendIcmpPackage(
    const int sockfd, 
    const std::string_view srcIpAddr, 
    const std::string_view dstIpAddr, 
    const unsigned int seqNum
)
{
    std::array<posnet::def::ByteType, 1024> buffer = {0};
    posnet::def::SizeType bufferSize = 0;
    posnet::IcmpBuilder icmpBuilder;
    struct sockaddr_in sockAddr;

    // Build sockaddr structure
    std::memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = inet_addr(dstIpAddr.data());

    // Build ICMP header
    icmpBuilder.setType(posnet::IcmpBuilder::PackageType::EchoRequest)
        .setCode(posnet::IcmpBuilder::PackageCode::None)
        .setId(getpid() & 0xFFFF)
        .setSequenceNumber(1)
        .setCheckSum(posnet::utils::CalcChecksum(icmpBuilder.getAsRawFrameView()));

    std::memcpy(buffer.data() + bufferSize, icmpBuilder.getStart(), icmpBuilder.getSize());
    bufferSize += icmpBuilder.getSize();

#ifdef DEBUG
    std::cout << icmpBuilder << std::endl;
#endif //! DEBUG
    return !(sendto(sockfd, buffer.data(), bufferSize, 0, (struct sockaddr*)&sockAddr, sizeof(sockAddr)) == -1);
}

void DumpFrame(
    const posnet::EthernetViewer& ethernet, 
    const posnet::IpViewer& ip, 
    const posnet::IcmpViewer& icmp, 
    const int verboseLevel
)
{
    std::stringstream ss;
    ss << "[*] " << "ip-address=" << ip.getSourceIpAddressAsStr() << "   ";
    if (verboseLevel > 0) {
        ss << "mac-address=" << ethernet.getSourceMacAddressAsStr();
    }

    LOG(ss.str());
}

void ParseIncomingIcmpPackages(const int verboseLevel, const int timeoutSec)
{
    int detectedHostNumber = 0;
    const auto sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        LOG_ERROR("Could not open socket for listening incoming icmp frames");
        return;
    }

    std::array<posnet::def::ByteType, 65536> buffer = {0};
    struct sockaddr sockaddr;
    socklen_t sockaddrSize;
    fd_set readfds;
    struct timeval tv;

    tv.tv_sec = timeoutSec;
    tv.tv_usec = 0;
    
    while (true) {
        std::memset(&sockaddr, 0, sockaddrSize);
        sockaddrSize = sizeof(sockaddr);

        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        const auto result = select(sockfd + 1, &readfds, nullptr, nullptr, &tv);
        if (result > 0) {
            const std::size_t bufferSize = recvfrom(sockfd, buffer.data(), buffer.size(), 0, &sockaddr, &sockaddrSize);
            if (bufferSize < 0) {
                LOG_ERROR("Could not read received package");
                return;
            }

            const posnet::EthernetViewer::ConstRawFrameViewType rawFrame{buffer.data(), bufferSize};
            const posnet::EthernetViewer ethernetViewer{rawFrame};
            if (ethernetViewer.getProtocol() == posnet::EthernetViewer::ProtocolType::IP) {
                const posnet::IpViewer ipViewer{ethernetViewer};
                if (ipViewer.getProtocol() == posnet::IpViewer::ProtocolType::ICMP) {
                    const posnet::IcmpViewer icmpViewer{ipViewer};
                    if (icmpViewer.getType() == posnet::IcmpViewer::PackageType::EchoReply) {
                        DumpFrame(ethernetViewer, ipViewer,  icmpViewer, verboseLevel);
                        ++detectedHostNumber;
                    }
                }
            }
        } else if (result == 0) {
            break;
        } else {
            LOG_ERROR("Could not wait for incoming net packages");
            return;
        }
    }

    std::stringstream ss;
    ss << "[*] Detected host number=" << detectedHostNumber << std::endl;
    ss << "===========================| Stop scanning |===========================" << std::endl;
    LOG(ss.str());
}

std::vector<std::string> SplitStr(const std::string_view str, const char delimiter) {
    std::vector<std::string> result;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string_view::npos) {
        result.push_back(std::string(str.substr(start, end - start)));
        start = end + 1;
        end = str.find(delimiter, start);
    }

    result.push_back(std::string(str.substr(start)));
    return result;
}


int main(int argc, char** argv)
{
    int verboseLevel = 0;
    int listenerWaitTimeoutSec = 1;
    std::set<std::string> scanIpAddrRange;
    std::string hostIpAddr;

    for (auto i = 1; i < argc; ++i) {
        const std::string_view option(argv[i]);

        if (const auto startPos = option.find(HELP_PARAM); startPos != std::string_view::npos) {
            Usage();
            return EXIT_SUCCESS;
        }

        if (const auto startPos = option.find(VERBOSE_LEVEL_PARAM); startPos != std::string_view::npos) {
            const auto levelStr = option.substr(startPos + VERBOSE_LEVEL_PARAM.size(), option.size() - startPos);
            verboseLevel = std::atoi(levelStr.data());
            continue;
        }

        if (const auto startPos = option.find(SCAN_IP_RANGE_PARAM); startPos != std::string_view::npos) {
            const auto rangeStr = option.substr(startPos + SCAN_IP_RANGE_PARAM.size(), option.size() - startPos);
            auto range = SplitStr(rangeStr, '-');
            //! TODO: fix GenerateIpAddrRange function
            scanIpAddrRange = posnet::utils::GenerateIpAddrRange(range.at(0), range.at(1));
            continue;
         }

        if (const auto startPos = option.find(WAIT_TIMEOUT_PARAM); startPos != std::string_view::npos) {
            const auto timeoutStr = option.substr(startPos + WAIT_TIMEOUT_PARAM.size(), option.size() - startPos);
            listenerWaitTimeoutSec = std::atoi(timeoutStr.data());
            continue;
        }

        if (const auto startPos = option.find(PING_PARAM); startPos != std::string_view::npos) {
            std::string pingIpAddrStr(option.substr(startPos + PING_PARAM.size(), option.size() - startPos));
            scanIpAddrRange.insert(std::move(pingIpAddrStr));
            continue;
         }
    }

    posnet::IFaceConfiguration::AddressType hostNetMaskAddr;
    unsigned int hostNetMaskAddrLen = 0;
    posnet::IFaceManager ifaceManager;

    const auto ifaceConfigs = ifaceManager.getConfigs();
    const auto it = std::find_if(ifaceConfigs.begin(), ifaceConfigs.end(), [](const auto& ifaceConfig) {
        return (ifaceConfig.getName() != posnet::IFaceConfiguration::LOOP_BACK_INTERFACE_NAME);
    });

    if (it == ifaceConfigs.cend()) {
        LOG_ERROR("Could not find no loopback net iface for scanning");
        return EXIT_FAILURE;
    }

    if (it->getIpAddress()) {
        hostIpAddr = *it->getIpAddress();
    } else {
        LOG_ERROR("Could not get host ip address");
        return EXIT_FAILURE;
    }

    if (scanIpAddrRange.empty()) {
        if (it->getNetMaskAddress()) {
            hostNetMaskAddr = *it->getNetMaskAddress();
        } else {
            LOG_ERROR("Could not get host ip address");
            return EXIT_FAILURE;
        }

        if (it->getNetMaskAddressLength()) {
            hostNetMaskAddrLen = *it->getNetMaskAddressLength();
        } else {
            LOG_ERROR("Could not get net mask address length");
            return EXIT_FAILURE;
        }

        //! TODO: fix this
        assert(false);
    }

    const auto sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    posnet::utils::ScopedLock socketLock([sockfd] {
        (void)close(sockfd);
    });

    if (sockfd == -1) {
        LOG_ERROR("Could not open socket");
        return EXIT_FAILURE;
    }

    LOG("===========================| Start scanning |===========================");

    std::thread listenerThread{ParseIncomingIcmpPackages, verboseLevel, listenerWaitTimeoutSec};
    posnet::utils::ScopedLock listenerThreadLock([&listenerThread] {
        listenerThread.join();
    });

    unsigned int seqNum = 0;
    for (const auto& targetIpAddr : scanIpAddrRange) {
        if (!SendIcmpPackage(sockfd, hostIpAddr, targetIpAddr, seqNum++)) {
            std::stringstream ss;
            ss << "Could not send the icmp package: ip-address=" << targetIpAddr << ", error_msg=" << strerror(errno);
            LOG_ERROR(ss.str());
        }
    }
    
    return EXIT_SUCCESS;
}