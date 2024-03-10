#include <iostream>
#include <iomanip>
#include <array>
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

#define DEBUG

int main() {   
    constexpr auto PORT = 12345;
    constexpr std::string_view payload("Hello, UDP server!");
    std::array<posnet::def::ByteType, 1024> buffer = {0};
    posnet::def::SizeType bufferSize = 0;

    // Set the destination address
    struct sockaddr_in sockAddr;
    std::memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    sockAddr.sin_port = htons(PORT);

    // Building frame
    {
        posnet::IpBuilder ipBuilder;
        ipBuilder.setHeaderLengthInBytes(posnet::IpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES)
                .setVersion(posnet::IpBuilder::VersionType::V4)
                .setTypeOfService(0)
                .setId(0)
                .setTTL(64)
                .setProtocol(posnet::IpBuilder::ProtocolType::UDP)
                .setSourceIpAddress("10.110.15.84")
                .setDestIpAddress("127.0.0.1");

        posnet::UdpBuilder udpBuilder;
        udpBuilder.setSourcePort(PORT + 1)
                .setDestPort(PORT)
                .setCheckSum(0) // for last frame is ok!
                .setUdpDataGramLength(posnet::UdpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + payload.size());

        ipBuilder.setTotalLength(posnet::IpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + 
                                    posnet::UdpBuilder::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + payload.size());
        ipBuilder.setCheckSum(posnet::utils::CalcChecksum(ipBuilder.getAsRawFrameView()));

        // filling the buffer
        std:memcpy(buffer.data() + 0, ipBuilder.getStart(), ipBuilder.getSize());
        std::memcpy(buffer.data() + ipBuilder.getSize(), udpBuilder.getStart(), udpBuilder.getSize());
        std::memcpy(buffer.data() + ipBuilder.getSize() + udpBuilder.getSize(), payload.data(), payload.size());
        bufferSize = ipBuilder.getSize() + udpBuilder.getSize() + payload.size();

#ifdef DEBUG
        std::cout << ipBuilder << std::endl;
        std::cout << udpBuilder << std::endl;
#endif //! DEBUG
    }

    // Create a raw socket
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sock == -1) {
        perror("socket");
        return 1;
    }

    // Set the option to include IP header (see https://www.opennet.ru/man.shtml?topic=raw&category=7&russian=0)
    {
        int one = 1;
        if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) == -1) {
            perror("setsockopt");
            close(sock);
            return 1;
        }
    }
    
    
    // Send the packet
    if (sendto(sock, buffer.data(), bufferSize, 0, (struct sockaddr*)&sockAddr, sizeof(sockAddr)) == -1) {
        perror("sendto");
        close(sock);
        return 1;
    }

    std::cout << "Sent the message successfully" << std::endl;
    // Close the socket
    return close(sock);
}
