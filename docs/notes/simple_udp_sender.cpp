#include <iostream>
#include <iomanip>
#include <cstring>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

// Function to calculate the checksum
uint16_t calculate_checksum(const uint16_t *buffer, size_t length) {
    uint32_t sum = 0;
    while (length > 1) {
        sum += *buffer++;
        length -= 2;
    }
    if (length > 0) {
        sum += *(uint8_t *)buffer;
    }
    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }
    return ~sum;
}

int main() {
    const int PORT = 12345; // Replace with your port number

    // Create a raw socket
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sock == -1) {
        perror("socket");
        return 1;
    }

    // Set the option to include IP header (see https://www.opennet.ru/man.shtml?topic=raw&category=7&russian=0)
    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) == -1) {
        perror("setsockopt");
        close(sock);
        return 1;
    }

    // Set the destination address
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Replace with the server IP address
    dest_addr.sin_port = htons(PORT); // Replace with the server port

    // Create the IP header
    struct iphdr ip_header;
    memset(&ip_header, 0, sizeof(ip_header));
    ip_header.ihl = 5;
    ip_header.version = 4;
    ip_header.tos = 0;
    ip_header.id = htons(0);
    ip_header.ttl = 64;
    ip_header.protocol = IPPROTO_UDP;
    ip_header.saddr = inet_addr("10.110.15.84"); // Replace with the source IP address
    ip_header.daddr = dest_addr.sin_addr.s_addr;
    // frag_off(0):my(32)?

    // Define the data to send
    const char* data = "Hello, UDP server!";

    // Calculate the UDP length
    uint16_t udp_length = htons(sizeof(struct udphdr) + strlen(data));
    std::cout << "udpLength=" << udp_length << std::endl;

    // Create the UDP header
    struct udphdr udp_header;
    memset(&udp_header, 0, sizeof(udp_header));
    udp_header.source = htons(PORT + 1); // Replace with the source port
    udp_header.dest = htons(PORT);
    udp_header.len = htons(sizeof(struct udphdr) + strlen(data)); // ***

    // Calculate the total length of the packet
    ip_header.tot_len = htons(sizeof(ip_header) + sizeof(struct udphdr) + strlen(data)); // ***

    // Calculate the IP checksum
    ip_header.check = htons(~calculate_checksum((uint16_t *)&ip_header, sizeof(ip_header))); // ***

    // Create the packet buffer
    posnet::def::ByteType packet[sizeof(ip_header) + udp_length];
    memcpy(packet, &ip_header, sizeof(ip_header));
    memcpy(packet + sizeof(ip_header), &udp_header, sizeof(udp_header));
    memcpy(packet + sizeof(ip_header) + sizeof(udp_header), data, strlen(data));

    // Send the packet
    if (sendto(sock, packet, sizeof(packet), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) == -1) {
        perror("sendto");
        close(sock);
        return 1;
    }

    std::cout << "Sent the message successfully" << std::endl;

    // Close the socket
    return close(sock);
} 