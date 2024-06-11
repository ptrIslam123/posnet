#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

int main() {
    std::string service_name = "_trueconf-airplayd-server._udp.local";
    addrinfo hints, *res;
    int sockfd;

    // Setup the hints for the getaddrinfo function
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    // Resolve the service name to an IP address
    int status = getaddrinfo(service_name.c_str(), NULL, &hints, &res);
    if (status != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return 2;
    }

    // Create a socket
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) {
        std::cerr << "socket error: " << strerror(errno) << std::endl;
        return 2;
    }

    // Send a UDP packet to the resolved address
    int numbytes = sendto(sockfd, "Hello, world!", 13, 0, res->ai_addr, res->ai_addrlen);
    if (numbytes == -1) {
        std::cerr << "sendto error: " << strerror(errno) << std::endl;
        return 2;
    }

    // Clean up
    freeaddrinfo(res);
    close(sockfd);

    return 0;
}
