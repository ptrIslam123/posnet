#include <iostream>
#include <array>
#include <vector>
#include <string>
#include <string_view>
#include <cstring>

#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "include/frame-viewers/dns_viewer.h"
#include "include/utils/sock_addr_convertor.h"
#include "include/utils/io/sync/streambuffer/istreambuffer.h"
#include "include/utils/scoped_lock.h"

constexpr auto SERVER_PORT = posnet::DnsViewer::DEFAULT_UDP_PORT;

int ParseDnsAnswer(const int sockfd)
{
    struct sockaddr_in sockAddr;
    std::memset(&sockAddr, 0, sizeof(sockAddr));
    socklen_t sockAddrLength = sizeof(sockAddr);

    std::array<posnet::def::ByteType, 1024> buffer = {0};
    const std::span<posnet::def::ByteType>::size_type readBytes =
            recvfrom(sockfd, buffer.data(), buffer.size(), 0, (struct sockaddr*)(&sockAddr), &sockAddrLength);
    if (readBytes < 0) {
        std::cerr << "Could not receive dns reply" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Received package from " << posnet::utils::v4::IpAddrToStr(sockAddr.sin_addr.s_addr) << std::endl;
    std::span<posnet::def::ByteType> dnsResponse{buffer.data(), readBytes};
    posnet::DnsViewer dnsViewer{ dnsResponse };
    std::cout << dnsViewer << std::endl;
    return EXIT_SUCCESS;
}

int main()
{
    const auto sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Could not open socket" << std::endl;
        return EXIT_FAILURE;
    }

    // Set up the server address structure
    struct sockaddr_in serverSockAddr;
    std::memset(&serverSockAddr, 0, sizeof(serverSockAddr));
    serverSockAddr.sin_family = AF_INET;
    serverSockAddr.sin_addr.s_addr = INADDR_ANY;
    serverSockAddr.sin_port = htons(SERVER_PORT);

    if (bind(sockfd, (struct sockaddr*)&serverSockAddr, sizeof(serverSockAddr)) < 0) {
        std::cerr << "Could not bind server socket: " << strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    posnet::utils::ScopedLock lock([sockfd] { close(sockfd); });
    while (true) {
        ParseDnsAnswer(sockfd);
    }
    return ParseDnsAnswer(sockfd);
}
