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
constexpr auto LOCAL_HOST_ADDR = "127.0.0.1";

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

void HandleWrite(
    const int sockfd,
    const std::string_view clientReply,
    struct sockaddr* clientSockAddr, 
    socklen_t clientSockAddrLength
) 
{
    auto writeBytes = 0;
    do {
        const auto size = sendto(sockfd, clientReply.data(), clientReply.size(), 0, clientSockAddr, clientSockAddrLength);
        if (size < 0) {
            std::cerr << "SERVER: write error" << std::endl;
            ioPoll.removeCallback(sockfd);
        } else if (size == 0) {
            // do nothing
        }
        writeBytes += size;
    } while (writeBytes < clientReply.size());

    ioPoll.removeCallback(sockfd, PollType::IOEventType::Write);
}

void HandleAccept(const int serverfd) {
    struct sockaddr_in clientSockAddr;
    std::memset(&clientSockAddr, 0, sizeof(clientSockAddr));
    socklen_t clientSockAddrLength = sizeof(clientSockAddr);

    std::array<char, 1024> buffer = {0};
    const auto readBytes = recvfrom(serverfd, buffer.data(), buffer.size(), 0, (struct sockaddr*)&clientSockAddr, &clientSockAddrLength);
    if (readBytes > 0) {
        const std::string_view clientData{ buffer.data(), static_cast<std::string_view::size_type>(readBytes) };
        std::cout << "SERVER: from=" << posnet::utils::v4::IpAddrToStr(clientSockAddr.sin_addr.s_addr)
            << ", data=" << clientData << std::endl;
        ioPoll.onWriteEvent(serverfd, [clientData, &clientSockAddr, &clientSockAddrLength](const int sockfd) mutable {
            HandleWrite(sockfd, clientData, (struct sockaddr*)&clientSockAddr, clientSockAddrLength);
        });
    } else if (readBytes == 0) {
        // do nothing
    } else {
        std::cerr << "SERVER: read error" << std::endl;
        ioPoll.removeCallback(serverfd);
    }
}

int main(int argc, char** argv) {
    posnet::utils::args::Parser parser(argc, argv);
    (void)parser.addArgPattern("--port")
        .addArgPattern("--help")
        .parse();

    if (parser.hasArg("--help")) {
        Usage(argv[0]);
        return EXIT_SUCCESS;
    }

    const int serverfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverfd < 0) {
        std::cerr << "SERVER: Could not create server socket" << std::endl;
        return EXIT_FAILURE;
    }

    posnet::utils::ScopedLock lock([serverfd] { close(serverfd); });

    const auto port = static_cast<unsigned short>(
        parser.getArgValue<int>("--port", '=').value_or(DEFAULT_PORT)
    );
    std::optional<std::uint32_t> hostIpAddr;
    {
        const auto ifaceConfig = posnet::GetFirstNonLoopbackIface();
        if (ifaceConfig && ifaceConfig->getIpAddress()) {
            const auto addr = posnet::utils::v4::StrToIpAddr(*ifaceConfig->getIpAddress());
            if (addr) {
                hostIpAddr = *addr;
            }
        }
    }

    struct sockaddr_in sockAddr;
    std::memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(port);
    sockAddr.sin_addr.s_addr = hostIpAddr ?  *hostIpAddr : htons(INADDR_ANY);

    // Bind the socket to the server address
    if (bind(serverfd, (struct sockaddr *)&sockAddr, sizeof(sockAddr)) < 0) {
        std::cerr << "SERVER: bind error: " << strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "SERVER: started udp server(pid=" << getpid() << ") (" << (hostIpAddr ? posnet::utils::v4::IpAddrToStr(*hostIpAddr) : LOCAL_HOST_ADDR) << " : " << std::to_string(port) << ")" << std::endl;
    ioPoll.onReadEvent(serverfd, HandleAccept);
    ioPoll.start();
    return EXIT_SUCCESS;
}
