#include <iostream>
#include <sstream>
#include <vector>
#include <thread>
#include <shared_mutex>
#include <set>
#include <algorithm>
#include <atomic>

#include <cstring>

#include <signal.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <features.h>   /* for the glibc version number */

#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1 
#include <netpacket/packet.h>
#include <net/ethernet.h>       /* the L2 protocols */
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>     /* The L2 protocols */ 
#endif

#include "include/frame-builder/ethernet_builder.h"
#include "include/frame-builder/arp_builder.h"
#include "include/frame-viewers/ethernet_viewer.h"
#include "include/frame-viewers/arp_viewer.h"

#include "include/net-iface/iface_manager.h"
#include "include/utils/io/sync/streambuffer/istreambuffer.h"
#include "include/utils/assert.h"
#include "include/utils/scoped_lock.h"
#include "include/utils/io/async/poll/poll.h"
#include "include/utils/args_parser.h"
#include "include/utils/algorithms.h"
#include "include/definitions.h"

constexpr auto DEFAULT_VERBOSE_LEVEL = 0;
constexpr auto DEFAULT_TIMOUT = 3;

using PollType = posnet::utils::io::async::posix::Poll; 
PollType ioPoll;

std::atomic<int> Verbose = 0;
std::atomic<int> ReceivedArpPackagesNumber = 0;
std::atomic<int> SentArpPackagesNumber = 0;
std::set<std::string> IpAddrSet;
std::shared_mutex IpAddrSetMutex;

void Usage(const std::string_view appName)
{
    std::stringstream ss;
    ss << "Usage" << "\n";
    ss << "\t\t" << appName << " [options]" << "\n";
    ss << "Options:" << "\n";
    ss << "\t--help                                         = Print help info/usage" << "\n";
    ss << "\t--verbose=<level>                              = Set up verbose level{0, 1}" << "\n";
    ss << "\t--range=[<start-ip-addr>-<end-ip-addr>]        = Set up ip address range for scanning" << "\n";
    ss << "\t--target=[target-ip-addresses]                 = Set up ip address for scanning" << "\n";
    ss << "\t--timeout=<value>                              = Waiting timeout of the icmp reply" << "\n";
    ss << "\t--listen                                       = Listen all local arp packages" << "\n";
    ss << "\t--timeout                                      = Set wait timeout(seconds)" << "\n";
    ss << std::endl;
    std::cout << ss.str() << std::endl;
}

void HandleSignal(const int signal)
{
    std::cout << "[*] RECEIVED signla=" << signal << "Stop process. " << std::endl;
    ioPoll.stop();
}

bool SendArpRequest(
    const int sockfd, 
    const struct sockaddr* sockAddr, 
    const socklen_t sockAddrLength, 
    const std::string_view srcMac, 
    const std::string_view srcIp, 
    const std::string_view targetIp
)
{
    posnet::EthernetBuilder ethernetBuilder;
    posnet::ArpBuilder arpBuilder;

    ethernetBuilder.setDestMacAddress(posnet::def::MAC_BROAD_CAST_ADDR)
            .setProtocol(posnet::EthernetBuilder::ProtocolType::ARP);

    arpBuilder.setHardwareType(posnet::ArpBuilder::DEFAULT_HARDWARE_ADDR_TYPE)
            .setProtocolType(posnet::ArpBuilder::DEFAULT_PROTOCOL_ADDR_TYPE)
            .setHardwareTypeLength(posnet::ArpBuilder::DEFAULT_HARDWARE_ADDR_TYPE_LENGTH)
            .setProtocolTypeLength(posnet::ArpBuilder::DEFAULT_PROTOCOL_ADDR_TYPE_LENGTH)
            .setOpcodeType(posnet::ArpBuilder::OpcodeType::ArpRequest)
            .setSourceMacAddress(srcMac)
            .setSourceIpAddress(srcIp)
            .setTargetIpAddress(targetIp);

    using namespace posnet::utils::io;
    posnet::utils::io::IStreamBuffer istreamBuffer;
    istreamBuffer << ethernetBuilder;
    istreamBuffer << arpBuilder;
    auto frameBuffer{ istreamBuffer.asSpan() };
    return (sendto(sockfd, frameBuffer.data(), frameBuffer.size(), 0, sockAddr, sockAddrLength) > 0);
}

void HandleArpReply(const int sockfd)
{
    std::set<std::string> targetIpAddrSet;
    {
        std::shared_lock lock(IpAddrSetMutex);
        targetIpAddrSet = IpAddrSet;        
    }

    std::array<posnet::def::ByteType, 65536> buffer = {0};
    struct sockaddr sockaddr;
    socklen_t sockaddrSize = sizeof(sockaddr);
    std::memset(&sockaddr, 0, sockaddrSize);
    
    const std::size_t readBytes = recvfrom(sockfd, buffer.data(), buffer.size(), 0, &sockaddr, &sockaddrSize);
    if (readBytes < 0) {
        std::cerr << "Failed to get packets" << std::endl;
        ioPoll.removeCallback(sockfd, PollType::IOEventType::Read);
        return;
    } else if (readBytes == 0) {
        std::cout << "Closed listener socket" << std::endl;
        ioPoll.removeCallback(sockfd, PollType::IOEventType::Read);
        return;
    }

    posnet::EthernetViewer ethernetViewer({ buffer.data(), readBytes });
    if (ethernetViewer.getProtocol() == posnet::EthernetViewer::ProtocolType::ARP) {
        ReceivedArpPackagesNumber.fetch_add(1);
        posnet::ArpViewer arpViewer(ethernetViewer);
        const auto senderIpAddr(arpViewer.getSenderIpAddressAsStr());
        if (!targetIpAddrSet.empty() && targetIpAddrSet.size() < 10) {
            if (targetIpAddrSet.find(senderIpAddr) == targetIpAddrSet.cend()) {
                return;
            }

            std::cout << "[*] Detected host: "
                << "ip-addr=" << arpViewer.getSenderIpAddressAsStr() << " | "
                << "mac-addr=" << arpViewer.getSenderMacAddressAsStr() << " | ";
            if (Verbose.load() > 2) {
                std::cout << "hardware-addr-type=" << arpViewer.getHardwareAddrTypeAsStr() << " | "
                    << "ip-addr-type=" << arpViewer.getProtocolAddrTypeAsStr() << " | "
                    << "opcode-type=" << arpViewer.getOpcodeTypeAsStr() << " | ";
            }
            return;
        } else {
            std::cout << "[*] Detected host: "
                << "ip-addr=" << arpViewer.getSenderIpAddressAsStr() << " | "
                << "mac-addr=" << arpViewer.getSenderMacAddressAsStr() << " | ";
            if (Verbose.load() > 2) {
                std::cout << "hardware-addr-type=" << arpViewer.getHardwareAddrTypeAsStr() << " | "
                    << "ip-addr-type=" << arpViewer.getProtocolAddrTypeAsStr() << " | "
                    << "opcode-type=" << arpViewer.getOpcodeTypeAsStr() << " | ";
            }
        }


        std::cout << std::endl;
        return;
    }
}

int main(int argc, char** argv)
{
    try {
        std::cout << "Started " << argv[0] << ", pid=" << getpid() << std::endl;

        const auto& iface = posnet::GetFirstNonLoopbackIface();
        ASSERTION(iface, std::runtime_error, "Did not detected non loopback iface")
        ASSERTION(iface->getMacAddress().has_value(), std::runtime_error, "Could not get host mac address")
        ASSERTION(iface->getIpAddress().has_value(), std::runtime_error, "Could not get host ip address")
        ASSERTION(iface->getNetMaskAddress().has_value(), std::runtime_error, "Could not get host net mask")
        ASSERTION(iface->getIndex().has_value(), std::runtime_error, "Could not get iface index")
        ASSERTION(iface->getNetMaskAddressLength().has_value(), std::runtime_error, "Could not get host sub net mask length")

        const auto hostMacAddr = *(iface->getMacAddress());
        const auto hostIpAddr = *(iface->getIpAddress());
        const auto hostNetMask = *(iface->getNetMaskAddress());
        const auto hostNetMaskLength  = *(iface->getNetMaskAddressLength());
        const auto hostIndex = *(iface->getIndex());

        posnet::utils::args::Parser parser(argc, argv);
        (void)parser.addArgPattern("--help")
                .addArgPattern("--verbose")
                .addArgPattern("--range")
                .addArgPattern("--target")
                .addArgPattern("--listen")
                .addArgPattern("--timeout")
                .parse();

        if (parser.hasArg("--help")) {
            Usage(argv[0]);
            return EXIT_SUCCESS;
        }

        if (parser.hasArg("--target") && parser.hasArg("--range")) {
            std::cerr << "You can not set argument --range and --target at same time" << std::endl;
            Usage(argv[0]);
            return EXIT_FAILURE;
        }

        Verbose.store(parser.getArgValue<int>("--verbose", ' ').value_or(DEFAULT_VERBOSE_LEVEL));
        auto timeout = parser.getArgValue<int>("--timeout", ' ').value_or(DEFAULT_TIMOUT);
        const auto rangeArgValue = parser.getArgValue<std::vector<std::string>>("--range",'-').
                value_or(std::vector<std::string>{});
        const auto targetArgValue = parser.getArgValue<std::vector<std::string>>("--target", ',').
                value_or(std::vector<std::string>{});

        if (!targetArgValue.empty()) {
            std::unique_lock lock(IpAddrSetMutex);
            for (auto&& ipAddr : targetArgValue) {
                IpAddrSet.insert(std::move(ipAddr));
            }
        } else if (!rangeArgValue.empty()) {
            if (rangeArgValue.size() != 2) {
                std::cerr << "Wrong range format" << std::endl;
                Usage(argv[0]);
                return EXIT_FAILURE;
            }

            std::unique_lock lock(IpAddrSetMutex);
            IpAddrSet = posnet::utils::GenerateIpAddrRange(rangeArgValue[0], rangeArgValue[1]);
        } else if (!parser.hasArg("--listen")) {
            std::unique_lock lock(IpAddrSetMutex);
            IpAddrSet = posnet::utils::GenerateIpAddrRange(hostIpAddr, hostNetMaskLength);
        }
        
        {
            std::shared_lock lock(IpAddrSetMutex);
            if (Verbose.load() > 0) {
                for (const auto& ipAddr : IpAddrSet) {
                    std::cout << "[VERBOSE=" << Verbose.load() << "]: target ip address for arp prequest=" << ipAddr << std::endl;
                }

                std::cout << std::endl;
            }
        }
        
        const auto sendSockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
        if(sendSockfd < 0) {
            std::cerr << "Could not create raw socket for sending arp package" << std::endl;
            return EXIT_FAILURE;
        }

        const auto listenSockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if (listenSockfd < 0) {
            std::cerr << "Could not create raw socket for listening arp package" << std::endl;
            return EXIT_FAILURE;
        }

        posnet::utils::ScopedLock lock([sendSockfd, listenSockfd]() {
            (void)close(sendSockfd);
            (void)close(listenSockfd);
        });

        ioPoll.onReadEvent(listenSockfd, HandleArpReply);
        std::thread listenerThread([]{
            ioPoll.start();
        });

        std::set<std::string> targetIpAddrSet;
        {
            std::shared_lock lock(IpAddrSetMutex);
            targetIpAddrSet = IpAddrSet;
        }

        const auto srcMacAddr(hostMacAddr);
        const auto srcIpAddr(hostIpAddr);
        for (const auto& targetIpAddr : targetIpAddrSet) {
            struct sockaddr_ll sockAddr;
            std::memset(&sockAddr, 0, sizeof(sockAddr));
            sockAddr.sll_family = PF_PACKET;
            sockAddr.sll_ifindex = hostIndex;
            socklen_t sockAddrLength = sizeof(sockAddr);

            if (SendArpRequest(sendSockfd, (const struct sockaddr*)&sockAddr, sockAddrLength, srcMacAddr, srcIpAddr, targetIpAddr)) {
                SentArpPackagesNumber.fetch_add(1);
            } else {
                std::cerr << "Could not send the arp package" << std::endl;
            }
        }

        if (!parser.hasArg("--listen")) {
            std::this_thread::sleep_for(std::chrono::seconds(300));
            ioPoll.stop();
        } else if (parser.hasArg("--listen")){
            
        }
        
        ASSERTION(listenerThread.joinable(), std::runtime_error, "Invalid listener thread handler")
        listenerThread.join();  
        
        std::cout << "[*]  Sent arp request=" << SentArpPackagesNumber.load() << ", Received arp reply=" << ReceivedArpPackagesNumber.load() << std::endl; 
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown error" << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
