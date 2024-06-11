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
#include "include/frame-builder/dns_builder.h"
#include "include/utils/sock_addr_convertor.h"
#include "include/utils/io/sync/streambuffer/istreambuffer.h"
#include "include/utils/args_parser.h"
#include "include/utils/scoped_lock.h"

auto DomainQuestion = std::string{"www.example.com"};
auto ServerIp = std::string{"8.8.8.8"}; // Google's public DNS server
auto ServerPort = posnet::DnsViewer::DEFAULT_UDP_PORT;
auto QuestionType = std::string{"a"};

int SendDnsQuestion(const int sockfd, const std::string_view domain, const std::string_view questionType)
{
    struct sockaddr_in serverSockAddr;
    std::memset(&serverSockAddr, 0, sizeof(serverSockAddr));
    serverSockAddr.sin_family = AF_INET;
    serverSockAddr.sin_port = htons(ServerPort);
    if (inet_pton(AF_INET, ServerIp.data(), &serverSockAddr.sin_addr) <= 0) {
        std::cerr << "Could not set up socket address" << std::endl;
        return EXIT_FAILURE;
    }

    const auto id = getpid();


    posnet::DnsBuilder::HeaderBuilder headerBuilder;
    std::vector<posnet::DnsBuilder::QuestionBuilder> questionBuilders;
    std::vector<posnet::DnsBuilder::AuthorityBuilder> authorityBuilders;
    // Set up header section
    {
        headerBuilder.setId(id)
            .setType(posnet::DnsBuilder::Type::Request)
            .setOpcode(posnet::DnsBuilder::OpcodeType::StandardQuery)
            .setFlagRD(posnet::DnsBuilder::TURN_ON_FLAG)
            .setQueryCount(1);
    }

    // Set up question section
    {
//        std::string queryName;
//        posnet::DnsBuilder::QueryType type;
//        if (questionType == "a") {
//            type = posnet::DnsBuilder::QueryType::HostAddress;
//            queryName = posnet::DnsBuilder::StrToDNSDomainFormat(domain);
//        } else if (questionType == "txt") {
//            type = posnet::DnsBuilder::QueryType::Text;
//            queryName = domain;
//        } else {
//            auto errorMsg = std::string("Does not support dns question type=") + std::string(questionType);
//            ASSERTION(false, std::runtime_error, errorMsg)
//        }


        auto n1 = posnet::DnsBuilder::StrToDNSDomainFormat("google.com");
        posnet::DnsBuilder::QuestionBuilder q1;
        q1.setType(posnet::DnsBuilder::QueryType::HostAddrIpV6)
                .setClass(posnet::DnsBuilder::QueryClass::Internet)
                .setName({(std::uint8_t*)n1.data(), n1.size()});
        questionBuilders.push_back(q1);

//        auto n2 = posnet::DnsBuilder::StrToDNSDomainFormat("www.example.com");
//        posnet::DnsBuilder::QuestionBuilder q2;
//        q2.setType(posnet::DnsBuilder::QueryType::HostAddrIpV4)
//                .setClass(posnet::DnsBuilder::QueryClass::Internet)
//                .setName({(std::uint8_t*)n2.data(), n2.size()});
//        questionBuilders.push_back(q2);
    }

    posnet::DnsBuilder dnsBuilder{ headerBuilder, questionBuilders };
    posnet::utils::io::IStreamBuffer istreamBuffer;
    istreamBuffer << dnsBuilder;
    auto buffer = istreamBuffer.asSpan();

    const auto writeBytes = sendto(sockfd, buffer.data(), buffer.size(), 0, (struct sockaddr *)&serverSockAddr, sizeof(serverSockAddr));
    if (writeBytes < 0) {
        std::cerr << "Could not send dns package" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Sent the package(id=" << id << ") to " << posnet::utils::v4::IpAddrToStr(serverSockAddr.sin_addr.s_addr) << std::endl;
    return EXIT_SUCCESS;
}

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

    std::cout << "Received reply from " << posnet::utils::v4::IpAddrToStr(sockAddr.sin_addr.s_addr) << std::endl;
    std::span<posnet::def::ByteType> dnsResponse{buffer.data(), readBytes};
    posnet::DnsViewer dnsViewer{ dnsResponse };
    std::cout << dnsViewer << std::endl;
    return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
    {
        posnet::utils::args::Parser parser(argc, argv);
        (void)parser.addArgPattern("--host")
                .addArgPattern("--domain")
                .addArgPattern("--type")
                .parse();

        DomainQuestion = parser.getArgValue<std::string>("--domain", '=').value_or(DomainQuestion);
        QuestionType = parser.getArgValue<std::string>("--type", '=').value_or(QuestionType);
        auto hostArgs = parser.getArgValue<std::vector<std::string>>("--host", ':')
                                                                    .value_or(std::vector<std::string>{
                                                                                  std::string{ ServerIp },
                                                                                  std::to_string(ServerPort)
                                                                              }
        );

        ASSERTION(hostArgs.size() >= 2, std::runtime_error, "Bad host arg")
        ServerIp = std::move(hostArgs[0]);
        ServerPort = std::atoi(hostArgs[1].data());
    }

    const auto sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Could not open socket" << std::endl;
        return EXIT_FAILURE;
    }

    posnet::utils::ScopedLock lock([sockfd] { close(sockfd); });
    SendDnsQuestion(sockfd, DomainQuestion, QuestionType);
    return ParseDnsAnswer(sockfd);
}
