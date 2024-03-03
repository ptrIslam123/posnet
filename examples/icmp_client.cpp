#include <iostream>
#include <string>
#include <string_view>
#include <array>

#include <cstring>

#include "include/frame-builder/ip_builder.h"
#include "include/frame-builder/icmp_builder.h"

#include "include/utils/algorithms.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

#define DEBUG

int main() {
    constexpr std::string_view destIpAddr("8.8.8.8"); 
    constexpr auto PORT = 12345;
    std::array<posnet::def::ByteType, 1024> buffer = {0};
    posnet::def::SizeType bufferSize = 0;

    // Create a raw socket for sending
    int send_sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (send_sock == -1) {
        perror("socket");
        return 1;
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
                .setSourceIpAddress("10.110.15.84")
                .setDestIpAddress(destIpAddr);

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
        std:memcpy(buffer.data() + 0, ipBuilder.getStart(), ipBuilder.getSize());
        std::memcpy(buffer.data() + ipBuilder.getSize(), icmpBuilder.getStart(), icmpBuilder.getSize());
        bufferSize = ipBuilder.getSize() + icmpBuilder.getSize();

#ifdef DEBUG
        std::cout << ipBuilder << std::endl;
        std::cout << icmpBuilder << std::endl;
#endif //! DEBUG
    }

    // Set the option to include IP header
    {
        int one = 1;
        if (setsockopt(send_sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) == -1) {
            perror("setsockopt");
            close(send_sock);
            return 1;
        }
    }

    // Set the destination address
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(destIpAddr.data());

    // Send the packet
    if (sendto(send_sock, buffer.data(), bufferSize, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) == -1) {
        perror("sendto");
        close(send_sock);
        return 1;
    }

    std::cout << "Sent the ICMP echo request successfully" << std::endl;

    // Close the sending socket
    return close(send_sock);
}