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

#include <array>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <cstring>

#include "include/frame-builder/ethernet_builder.h"
#include "include/frame-builder/arp_builder.h"

#include "include/net-iface/iface_manager.h"

#include "include/utils/scoped_lock.h"
#include "include/definitions.h"

void SendArpRequest(
    const int sockfd, const struct sockaddr* sockAddr, 
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
    if(sendto(sockfd, frameBuffer.data(), frameBuffer.size(), 0, sockAddr, sockAddrLength) <= 0) {
        std::cerr << "Could not send the arp package" << std::endl;
    } else {
        std::cout << "Sent the arp package" << std::endl;
    }
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cerr << "Pass wrong args. Use <target-ip-addr> for arp request" << std::endl;
        return EXIT_SUCCESS;
    }

    const std::string_view targetIpAddr(argv[1]);
    const auto sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if(sockfd < 0) {
        std::cerr << "Could not create raw socket" << std::endl;
        return EXIT_FAILURE;
    }

    posnet::utils::ScopedLock lock([sockfd]() {
        (void)close(sockfd);
    });

    std::string hostIpAddr;
    std::string hostMacAddr;
    struct sockaddr_ll sockAddr;
    socklen_t sockAddrLength = sizeof(sockAddr);
    {
        std::memset(&sockAddr, 0, sizeof(sockAddr));
        sockAddr.sll_family = PF_PACKET;
        
        posnet::IFaceManager ifaceManager;
        const auto& configs = ifaceManager.getConfigs();
        const auto it = std::find_if(configs.cbegin(), configs.cend(), [](const auto& config) {
            return (config.getName() && *config.getName() != posnet::IFaceConfiguration::LOOP_BACK_INTERFACE_NAME);
        });

        if (it == configs.cend()) {
            std::cerr << "Could not find no loopback iface" << std::endl;
            return EXIT_FAILURE;
        }

        if (it->getIndex()) {
            sockAddr.sll_ifindex = *it->getIndex();
        } else {
            std::cerr << "Could not get appropriate iface index" << std::endl;
            return EXIT_FAILURE;
        }

        if (it->getIpAddress()) {
            hostIpAddr = *it->getIpAddress();
        } else {
            std::cerr << "Could not get host ip address" << std::endl;
            return EXIT_FAILURE;
        }

        if (it->getMacAddress()) {
            hostMacAddr = *it->getMacAddress();
        } else {
            std::cerr << "Could not get hos mac address" << std::endl;
            return EXIT_FAILURE;
        }
    }
    
    try {
        SendArpRequest(sockfd, (struct sockaddr*)&sockAddr, sockAddrLength, hostMacAddr, hostIpAddr, targetIpAddr);
    } catch (posnet::BadEthernetPackage &e) {
        std::cerr << "BadEthernetPackage: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (posnet::BadArpPackage& e) {
        std::cerr << "BadArpPackage: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;    
}
