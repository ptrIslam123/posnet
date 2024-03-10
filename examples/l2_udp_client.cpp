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
#include "include/frame-builder/udp_builder.h"

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
    constexpr std::string_view PAYLOAD("Hello, UDP server!");
    constexpr auto PORT = 12345;

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
                .setDestMacAddress("58:11:22:06:96:00")
                .setSourceMacAddress(myMacAddr);

        posnet::IpBuilder ipBuilder;
        ipBuilder.setHeaderLengthInBytes(posnet::IpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES)
                .setVersion(posnet::IpBuilder::VersionType::V4)
                .setTypeOfService(0)
                .setId(0)
                .setTTL(64)
                .setProtocol(posnet::IpBuilder::ProtocolType::UDP)
                .setSourceIpAddress(myIpAddr)
                .setDestIpAddress(myIpAddr);

        posnet::UdpBuilder udpBuilder;
        udpBuilder.setSourcePort(PORT + 1)
                .setDestPort(PORT)
                .setCheckSum(0) // for last frame is ok!
                .setUdpDataGramLength(posnet::UdpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + PAYLOAD.size());

        ipBuilder.setTotalLength(posnet::IpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + 
                                    posnet::UdpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + PAYLOAD.size());
        ipBuilder.setCheckSum(posnet::utils::CalcChecksum(ipBuilder.getAsRawFrameView()));


        // filling the buffer
        std::memcpy(buffer.data() + bufferSize, ethernetBuilder.getStart(), ethernetBuilder.getSize());
        bufferSize += ethernetBuilder.getSize();

        std::memcpy(buffer.data() + bufferSize, ipBuilder.getStart(), ipBuilder.getSize());
        bufferSize += ipBuilder.getSize();

        std::memcpy(buffer.data() + bufferSize, udpBuilder.getStart(), udpBuilder.getSize());
        bufferSize += udpBuilder.getSize();

        std::memcmp(buffer.data() + bufferSize, PAYLOAD.data(), PAYLOAD.size());
        bufferSize + PAYLOAD.size();

#ifdef DEBUG
        std::cout << ethernetBuilder << std::endl;
        std::cout << ipBuilder << std::endl;
        std::cout << udpBuilder << std::endl;
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