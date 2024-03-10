#include <iostream>
#include <string>
#include <string_view>
#include <array>
#include <algorithm>

#include <cstring>
#include <cassert>

#include "include/net-iface/iface_manager.h"

#include "include/frame-builder/ip_builder.h"
#include "include/frame-builder/icmp_builder.h"

#include "include/utils/algorithms.h"
#include "include/utils/scoped_lock.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

#define DEBUG

int main() {
    // public ip address of goog`s dns service
    constexpr std::string_view GOOG_DNS_SERVER_IP_ADDR("8.8.8.8"); 
    constexpr auto PORT = 12345;

    std::array<posnet::def::ByteType, 1024> buffer = {0};
    posnet::def::SizeType bufferSize = 0;
    struct sockaddr_in sockAddr;
    std::string myIpAddr;

    // Set up socket address structure
    {
        posnet::IFaceManager ifaceManager;
        const auto configs = ifaceManager.getConfigs();
        const auto it = std::find_if(configs.cbegin(), configs.cend(), [](const posnet::IFaceConfiguration& config) {
            return (config.getName() && (*config.getName() != posnet::IFaceConfiguration::LOOP_BACK_INTERFACE_NAME));
        });

        if (it == configs.cend()) {
            return EXIT_FAILURE;
        }

        assert(it->getIpAddress());
        myIpAddr = *it->getIpAddress();

        
        memset(&sockAddr, 0, sizeof(sockAddr));
        sockAddr.sin_family = AF_INET;
        sockAddr.sin_addr.s_addr = inet_addr(myIpAddr.data());
    }

    // Building frame
    {
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
        std:memcpy(buffer.data() + bufferSize, ipBuilder.getStart(), ipBuilder.getSize());
        bufferSize += ipBuilder.getSize();

        std::memcpy(buffer.data() + bufferSize, icmpBuilder.getStart(), icmpBuilder.getSize());
        bufferSize += icmpBuilder.getSize();

#ifdef DEBUG
        std::cout << ipBuilder << std::endl;
        std::cout << icmpBuilder << std::endl;
#endif //! DEBUG
    }

    // Create a raw socket for sending
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    posnet::utils::ScopedLock socketLock([sock] {
        (void)close(sock);
    });

    if (sock == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    // Set the option to include IP header
    {
        int one = 1;
        if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) == -1) {
            perror("setsockopt");
            return EXIT_FAILURE;
        }
    }

    // Send the packet
    if (sendto(sock, buffer.data(), bufferSize, 0, (struct sockaddr*)&sockAddr, sizeof(sockAddr)) == -1) {
        perror("sendto");
        return EXIT_FAILURE;
    }

    std::cout << "Sent the ICMP echo request successfully" << std::endl;
    return EXIT_SUCCESS;
}