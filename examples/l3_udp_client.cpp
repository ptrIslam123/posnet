#include <iostream>
#include <array>
#include <algorithm>
#include <cassert>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

#include "include/net-iface/iface_manager.h"

#include "include/frame-builder/ip_builder.h"
#include "include/frame-builder/udp_builder.h"

#include "include/frame-viewers/ip_viewer.h"
#include "include/frame-viewers/udp_viewer.h"

#include "include/utils/algorithms.h"
#include "include/utils/scoped_lock.h"

#define DEBUG

int main() {   
    constexpr auto PORT = 12345;
    constexpr std::string_view PAYLOAD("Hello, UDP server!");

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

        std::memset(&sockAddr, 0, sizeof(sockAddr));
        sockAddr.sin_family = AF_INET;
        sockAddr.sin_addr.s_addr = inet_addr(myIpAddr.data());
        sockAddr.sin_port = htons(PORT);
    }  

    // Building frame
    {
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
                .setCheckSum(0)
                .setUdpDataGramLength(posnet::UdpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + PAYLOAD.size());

        ipBuilder.setTotalLength(posnet::IpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + 
                                    posnet::UdpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + PAYLOAD.size());
        ipBuilder.setCheckSum(posnet::utils::CalcChecksum(ipBuilder.getAsRawFrameView()));

        // filling the buffer
        std:memcpy(buffer.data() + bufferSize, ipBuilder.getStart(), ipBuilder.getSize());
        bufferSize += ipBuilder.getSize();

        std::memcpy(buffer.data() + bufferSize, udpBuilder.getStart(), udpBuilder.getSize());
        bufferSize += udpBuilder.getSize();

        std::memcpy(buffer.data() + bufferSize, PAYLOAD.data(), PAYLOAD.size());
        bufferSize += PAYLOAD.size();

#ifdef DEBUG
        std::cout << ipBuilder << std::endl;
        std::cout << udpBuilder << std::endl;
#endif //! DEBUG
    }

    // Create a raw socket
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    posnet::utils::ScopedLock socketLock([sock] {
        (void)close(sock);
    });

    if (sock == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    // Set the option to include IP header (see https://www.opennet.ru/man.shtml?topic=raw&category=7&russian=0)
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

    std::cout << "Sent the message successfully" << std::endl;
    return EXIT_SUCCESS;
}
