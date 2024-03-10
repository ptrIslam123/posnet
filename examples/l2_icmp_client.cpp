#include <iostream>
#include <string>
#include <string_view>
#include <array>
#include <algorithm>

#include <cstring>
#include <cassert>

#include "include/net-iface/iface_manager.h"

#include "include/frame-builder/ethernet_builder.h"
#include "include/frame-builder/ip_builder.h"
#include "include/frame-builder/icmp_builder.h"

#include "include/utils/algorithms.h"
#include "include/utils/scoped_lock.h"

#include <linux/if_packet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/ethernet.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

#define DEBUG

int main() {
    // determined goog`s dns service mac address through parsing of icmp reply(ping)
    constexpr std::string_view GOOG_DNS_SERVER_MAC_ADDR("0e:74:ac:78:44:b7");
    // public ip address 
    constexpr std::string_view GOOG_DNS_SERVER_IP_ADDR("8.8.8.8");

    std::array<posnet::def::ByteType, 1024> buffer = {0};
    posnet::def::SizeType bufferSize = 0;
    
    std::string myMacAddr;
    std::string myIpAddr;

    // Set up socket address structure
    struct sockaddr_ll sockAddr;
    {
        posnet::IFaceManager ifaceManager;
        const auto configs = ifaceManager.getConfigs();
        const auto it = std::find_if(configs.cbegin(), configs.cend(), [](const posnet::IFaceConfiguration& config) {
            return (config.getName() && (*config.getName() != posnet::IFaceConfiguration::LOOP_BACK_INTERFACE_NAME));
        });

        if (it == configs.cend()) {
            return 1;
        }

        assert(it->getIndex());
        assert(it->getMacAddress());
        assert(it->getIpAddress());

        std::memset(&sockAddr, 0, sizeof(sockAddr));
        sockAddr.sll_family = AF_PACKET;
        sockAddr.sll_protocol = htons(ETH_P_IP);
        sockAddr.sll_ifindex = *it->getIndex();

        myMacAddr = *it->getMacAddress();
        myIpAddr = *it->getIpAddress();
    }

    // Building frame
    {
        posnet::EthernetBuilder ethernetBuilder;
        ethernetBuilder.setProtocol(posnet::EthernetBuilder::ProtocolType::IP)
                .setDestMacAddress(GOOG_DNS_SERVER_MAC_ADDR)
                .setSourceMacAddress(myMacAddr);

        posnet::IpBuilder ipBuilder;
        ipBuilder.setHeaderLengthInBytes(posnet::IpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES)
                .setVersion(posnet::IpBuilder::VersionType::V4)
                .setTypeOfService(0)
                .setId(0)
                .setTTL(64)
                .setProtocol(posnet::IpBuilder::ProtocolType::ICMP)
                .setSourceIpAddress(myIpAddr)
                .setDestIpAddress(GOOG_DNS_SERVER_IP_ADDR);

        posnet::IcmpBuilder icmpBuilder;
        icmpBuilder.setType(posnet::IcmpBuilder::PackageType::EchoRequest)
            .setCode(posnet::IcmpBuilder::PackageCode::None)
            .setId(getpid() & 0xFFFF)
            .setSequenceNumber(1)
            .setCheckSum(posnet::utils::CalcChecksum(icmpBuilder.getAsRawFrameView()));

        ipBuilder.setTotalLength(posnet::IpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + 
                                    posnet::IcmpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES);
        ipBuilder.setCheckSum(posnet::utils::CalcChecksum(ipBuilder.getAsRawFrameView()));

        // filling the buffer
        std::memcpy(buffer.data() + bufferSize, ethernetBuilder.getStart(), ethernetBuilder.getSize());
        bufferSize += ethernetBuilder.getSize();

        std::memcpy(buffer.data() + bufferSize, ipBuilder.getStart(), ipBuilder.getSize());
        bufferSize += ipBuilder.getSize();

        std::memcpy(buffer.data() + bufferSize, icmpBuilder.getStart(), icmpBuilder.getSize());
        bufferSize += icmpBuilder.getSize();

#ifdef DEBUG
        std::cout << ethernetBuilder << std::endl;
        std::cout << ipBuilder << std::endl;
        std::cout << icmpBuilder << std::endl;
#endif //! DEBUG
    }

     // Create a raw socket for sending
    int sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    posnet::utils::ScopedLock socketLock([sock]{
        (void)close(sock);
    });

    if (sock == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }
    
    // Send the packet
    if (sendto(sock, buffer.data(), bufferSize, 0, (struct sockaddr*)&sockAddr, sizeof(sockAddr)) == -1) {
        perror("sendto");
        return EXIT_FAILURE;
    }

    std::cout << "Sent the Udp request successfully" << std::endl;
    return EXIT_SUCCESS;
}