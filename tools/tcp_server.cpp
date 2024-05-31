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

#include "include/utils/args_parser.h"
#include "include/utils/scoped_lock.h"
#include "include/utils/sock_addr_convertor.h"
#include "include/utils/io/async/poll/poll.h"
#include "include/net-iface/iface_manager.h"

constexpr auto DEFAULT_PORT = 12345;

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

void HandleClintWrite(const int sockfd, const std::string_view reply)
{
    const auto writeBytes = send(sockfd, reply.data(), reply.size(), 0);
    if (writeBytes < 0) {
        std::cerr << "SERVER: write error" << strerror(errno) << std::endl;
        ioPoll.removeCallback(sockfd, PollType::IOEventType::ReadWrite);
    }else if (writeBytes == 0) {
        std::cout << "SERVER: closed client socket" << std::endl;
        ioPoll.removeCallback(sockfd, PollType::IOEventType::Write);
    } else {
        assert(writeBytes == reply.size());
        //! Do nothing
    }

    ioPoll.removeCallback(sockfd, PollType::IOEventType::Write);
}

void HandleClientRead(const int sockfd)
{
    struct sockaddr_in clientSockAddr;
    std::memset(&clientSockAddr, 0, sizeof(clientSockAddr));
    socklen_t clientSockAddrLength = sizeof(clientSockAddr);

    std::array<char, 1024> buffer = {0};
    const auto readBytes = recvfrom(sockfd, buffer.data(), buffer.size(), 0, (struct sockaddr*)&clientSockAddr, &clientSockAddrLength);
    if (readBytes > 0) {
        const std::string_view clientData{ buffer.data(), static_cast<std::string_view::size_type>(readBytes) };
        std::cout << "SERVER: from=" << posnet::utils::IpAddrToStr(clientSockAddr.sin_addr.s_addr)
            << ", data=" << clientData << std::endl;
        ioPoll.onWriteEvent(sockfd,  [clientData](const int sockfd) {
            HandleClintWrite(sockfd, clientData);
        });
    } else if (readBytes == 0) {
        std::cout << "SERVER: closed client: " << posnet::utils::IpAddrToStr(clientSockAddr.sin_addr.s_addr) << std::endl;
        ioPoll.removeCallback(sockfd, PollType::IOEventType::Read);
    } else {
        std::cerr << "SERVER: read error " << strerror(errno) << std::endl;
        ioPoll.removeCallback(sockfd, PollType::IOEventType::ReadWrite);
    }
}

void AcceptClient(const int serverSockfd)
{
    struct sockaddr_in clientSockAddr;
    std::memset(&clientSockAddr, 0, sizeof(clientSockAddr));
    socklen_t clientSockAddrLength = sizeof(clientSockAddr);
    int clientfd = 0;

    if ((clientfd = accept(serverSockfd, (struct sockaddr*)&clientSockAddr, (socklen_t*)&clientSockAddrLength)) < 0) {
        std::cerr << "SERVER: accept failed";
    } else {
        std::cout << "SERVER: accepted a new client: "
                << "(ip=" <<  posnet::utils::IpAddrToStr(clientSockAddr.sin_addr.s_addr) << " : "
                << "port=" << clientSockAddr.sin_port << ")" << std::endl;

        ioPoll.onReadEvent(clientfd, HandleClientRead);
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

    int sockfd;
    struct sockaddr_in serverSockAddr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "SERVER: could not open server socket" << std::endl;
        return EXIT_FAILURE;
    }

    posnet::utils::ScopedLock lock([sockfd] {
        close(sockfd);
        std::cout << "SERVER: closed server socket" << std::endl;
    });

    {
        int opt = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
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
    if (bind(sockfd, (const struct sockaddr *)&serverSockAddr, sizeof(serverSockAddr)) < 0) {
        std::cerr << "SEVER: could not bind server socket to address and port" << std::endl;
        return EXIT_FAILURE;
    }

    // Listen for incoming connections
    if (listen(sockfd, 3) < 0) {
        std::cerr << "SERVER: could not listen socket for accepting incoming client connection" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "SERVER: started tcp server(pid=" << getpid() << ") (" << (hostIpAddr ? posnet::utils::IpAddrToStr(*hostIpAddr) : "127.0.0.1") << " : " << std::to_string(port) << ")" << std::endl;
    ioPoll.onReadEvent(sockfd, AcceptClient);
    ioPoll.start();
    return EXIT_SUCCESS;
}
