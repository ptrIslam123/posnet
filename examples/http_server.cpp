#include <iostream>
#include <array>
#include <string>
#include <string_view>

#include <cstring>
#include <cassert>
#include <cstdint>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "include/net-iface/iface_manager.h"

#include "include/utils/sock_addr_convertor.h"
#include "include/utils/io/async/poll/poll.h"
#include "include/utils/scoped_lock.h"
#include "include/utils/args_parser.h"

constexpr auto DEFAULT_PORT = 4000;

using PollType = posnet::utils::io::async::posix::Poll;
PollType ioPoll;


void Usage(const std::string_view appName)
{
    std::stringstream ss;
    ss << "Usage" << "\n";
    ss << "\t" << appName << " [options]" << "\n";
    ss << "Options:" << "\n";
    ss << "\t--help                                         = Print help info/usage" << "\n";
    ss << "\t--port=<port-number>                           = Set up start server port" << "\n";
    ss << std::endl;
    std::cout << ss.str() << std::endl;
}

void ProcessHttpRequest(const int clientfd, const std::string_view clientIpAddr, const std::string_view request) {
    std::cout << "from=" << clientIpAddr << ", request=" << request << std::endl;

    const std::string_view response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, World!";

    send(clientfd, response.data(), response.size(), 0);
}

void HandleClientRequest(const int clientfd) {
    struct sockaddr_in clientSockAddr;
    std::memset(&clientSockAddr, 0, sizeof(clientSockAddr));
    socklen_t clientSockAddrLength = sizeof(clientSockAddr);

    std::array<char, 1024> buffer = {0};
    const auto readBytes = recvfrom(clientfd, buffer.data(), buffer.size(), 0, (struct sockaddr*)&clientSockAddr, &clientSockAddrLength);
    const auto clientAddr = posnet::utils::IpAddrToStr(clientSockAddr.sin_addr.s_addr);
    if (readBytes > 0) {
        buffer[readBytes] = '\0';
        ProcessHttpRequest(clientfd, clientAddr, std::string_view{ buffer.data(), static_cast<std::string_view::size_type>(readBytes) });
    } else if (readBytes == 0) {
        std::cout << "SERVER: closed client connection" << std::endl;
        ioPoll.removeCallback(clientfd);
    } else {
        std::cerr << "SERVER: read error" << std::endl;
        ioPoll.removeCallback(clientfd);
    }
} 

void HandleAccept(const int serverfd) {
    struct sockaddr_in clientSockAddr;
    std::memset(&clientSockAddr, 0, sizeof(clientSockAddr));
    socklen_t clientSockAddrLength = sizeof(clientSockAddr);
    int clientfd = 0;

    if ((clientfd = accept(serverfd, (struct sockaddr*)&clientSockAddr, (socklen_t*)&clientSockAddrLength)) < 0) {
        std::cerr << "SERVER: accept failed";
    } else {
        std::cout << "SERVER: accepted a new client: "
                << "(ip=" <<  posnet::utils::IpAddrToStr(clientSockAddr.sin_addr.s_addr) << " : "
                << "port=" << clientSockAddr.sin_port << ")" << std::endl;

        ioPoll.onReadEvent(clientfd, HandleClientRequest);
    }
}

int main(int argc, char** argv)
{
    posnet::utils::args::Parser parser(argc, argv);
    (void)parser.addArgPattern("--port")
        .addArgPattern("--help")
        .parse();

    if (parser.hasArg("--help")) {
        Usage(argv[0]);
        return EXIT_SUCCESS;
    }

    const auto port = static_cast<unsigned short>(
        parser.getArgValue<int>("--port", '=').value_or(DEFAULT_PORT)
    );
    std::optional<std::uint32_t> hostIpAddr;
    {
        const auto ifaceConfig = posnet::GetFirstNonLoopbackIface();
        if (ifaceConfig && ifaceConfig->getIpAddress()) {
            const auto addr = posnet::utils::StrToIpAddr(*ifaceConfig->getIpAddress());
            if (addr) {
                hostIpAddr = *addr;
            }
        }
    }

    int serverfd;
    struct sockaddr_in serverSockAddr;
    if ((serverfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "SERVER: could not open server socket" << std::endl;
        return EXIT_FAILURE;
    }

    posnet::utils::ScopedLock lock([serverfd] {
        close(serverfd);
        std::cout << "SERVER: closed server socket" << std::endl;
    });

    {
        int opt = 1;
        if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
            std::cerr << "SERVER: could not set socket option(reuse address | reuse port)" << std::endl;
            return EXIT_FAILURE;
        }
    }

    // Set up the server address structure
    std::memset(&serverSockAddr, 0, sizeof(serverSockAddr));
    serverSockAddr.sin_family = AF_INET;
    serverSockAddr.sin_addr.s_addr = (hostIpAddr ? *hostIpAddr : INADDR_ANY);
    serverSockAddr.sin_port = htons(port);

    // Bind the socket to the address
    if (bind(serverfd, (const struct sockaddr *)&serverSockAddr, sizeof(serverSockAddr)) < 0) {
        std::cerr << "SEVER: could not bind server socket to address and port" << std::endl;
        return EXIT_FAILURE;
    }

    // Listen for incoming connections
    if (listen(serverfd, 3) < 0) {
        std::cerr << "SERVER: could not listen socket for accepting incoming client connection" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "SERVER: started tcp server(pid=" << getpid() << ") (" << (hostIpAddr ? posnet::utils::IpAddrToStr(*hostIpAddr) : "127.0.0.1") << " : " << std::to_string(port) << ")" << std::endl;
    ioPoll.onReadEvent(serverfd, HandleAccept);
    ioPoll.start();
    return 0;
}