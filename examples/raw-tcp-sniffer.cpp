#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>

// Function to print packet information
void print_packet_info(const u_char *packet, int packet_size) {
    struct ethhdr *eth = (struct ethhdr *)packet;
    if (ntohs(eth->h_proto) != ETH_P_IP) {
        return; // Not an IP packet
    }

    struct iphdr *iph = (struct iphdr *)(packet + sizeof(struct ethhdr));
    if (iph->protocol != IPPROTO_TCP) {
        return; // Not a TCP packet
    }

    struct tcphdr *tcph = (struct tcphdr *)(packet + sizeof(struct ethhdr) + iph->ihl * 4);

    char source_ip[INET_ADDRSTRLEN];
    char dest_ip[INET_ADDRSTRLEN];

    strncpy(source_ip, inet_ntoa(*(struct in_addr *)&iph->saddr), INET_ADDRSTRLEN);
    strncpy(dest_ip, inet_ntoa(*(struct in_addr *)&iph->daddr), INET_ADDRSTRLEN);

    std::cout << "Source IP: " << source_ip << std::endl;
    std::cout << "Destination IP: " << dest_ip << std::endl;
    std::cout << "Source Port: " << ntohs(tcph->source) << std::endl;
    std::cout << "Destination Port: " << ntohs(tcph->dest) << std::endl;
    std::cout << "Packet Size: " << packet_size << " bytes" << std::endl;
    std::cout << std::endl;
}

int main() {
    int sock_raw;
    socklen_t saddr_size;
    struct sockaddr saddr;
    unsigned char *buffer = new unsigned char[65536]; // Adjust buffer size as needed

    // Create a raw socket
    sock_raw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock_raw < 0) {
        perror("Socket Error");
        return -1;
    }

    while (true) {
        saddr_size = sizeof(saddr);
        int data_size = recvfrom(sock_raw, buffer, 65536, 0, &saddr, &saddr_size);
        if (data_size < 0) {
            std::cerr << "Failed to get packets" << std::endl;
            return -1;
        }

        print_packet_info(buffer, data_size);
    }

    close(sock_raw);
    delete[] buffer;

    return 0;
}