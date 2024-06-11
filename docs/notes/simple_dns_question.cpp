#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstring>
#include <array>
#include <vector>
#include <string>
#include <string_view>

#define DNS_SERVER_IP "8.8.8.8" // Google's public DNS server
#define DNS_SERVER_PORT 53

/*

...
User Datagram Protocol, Src Port: 46034, Dst Port: 53
Domain Name System (query)
    Transaction ID: 0xde70
    Flags: 0x0100 Standard query
        0... .... .... .... = Response: Message is a query
        .000 0... .... .... = Opcode: Standard query (0)
        .... ..0. .... .... = Truncated: Message is not truncated
        .... ...1 .... .... = Recursion desired: Do query recursively
        .... .... .0.. .... = Z: reserved (0)
        .... .... ...0 .... = Non-authenticated data: Unacceptable
    Questions: 1
    Answer RRs: 0
    Authority RRs: 0
    Additional RRs: 0
    Queries
        google.com: type A, class IN
            Name: google.com
            [Name Length: 10]
            [Label Count: 2]
            Type: A (Host Address) (1)
            Class: IN (0x0001)
    [Response In: 2]

*/

// DNS header structure
typedef struct {
    uint16_t id;
    //uint16_t flags;
    uint8_t rd :1;
    uint8_t tc :1;
    uint8_t aa :1;
    uint8_t opcode :4;
    uint8_t qr :1;
    uint8_t rcode :4;
    uint8_t z :3;
    uint8_t ra :1;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
} DNS_HEADER;

// DNS question structure
typedef struct {
    uint16_t qtype;
    uint16_t qclass;
} DNS_QUESTION;

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    DNS_HEADER dns_header;
    DNS_QUESTION dns_question;
    unsigned char dns_query[512];
    ssize_t bytes_sent;

    // Create a new socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // Set up the server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DNS_SERVER_PORT);
    if (inet_pton(AF_INET, DNS_SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Set up the DNS header
    dns_header.id = htons(getpid());
    dns_header.qr = 0; // This is a query
    dns_header.opcode = 0; // This is a standard query
    dns_header.aa = 0;
    dns_header.tc = 0;
    dns_header.rd = 1; // Recursion desired
    dns_header.ra = 0;
    dns_header.z = 0;
    dns_header.rcode = 0;
    dns_header.qdcount = htons(1); // We have one question
    dns_header.ancount = 0;
    dns_header.nscount = 0;
    dns_header.arcount = 0;

    // Set up the DNS question
    dns_question.qtype = htons(1); // Type A (IPv4 address)
    dns_question.qclass = htons(1); // Class IN (Internet)

    // Create the DNS 34
    memset(dns_query, 0, sizeof(dns_query));
    unsigned char *query_ptr = dns_query;
    memcpy(query_ptr, &dns_header, sizeof(dns_header));
    query_ptr += sizeof(dns_header);

    // Pack the domain name into DNS format
    const char *domain = "google.com";// "www.example.com"; // ok
    const char *label = domain;
    while (*label) {
        const char *next_dot = strchr(label, '.');
        uint8_t label_length = next_dot ? next_dot - label : strlen(label);
        *query_ptr++ = label_length;
        memcpy(query_ptr, label, label_length);
        query_ptr += label_length;
        label += label_length;
        if (*label == '.') {
            label++; // Skip the dot
        }
    }
    *query_ptr++ = 0; // Null label to indicate the end of the name

    // Add the question
    memcpy(query_ptr, &dns_question, sizeof(dns_question));
    query_ptr += sizeof(dns_question);

    // Send the DNS query
    bytes_sent = sendto(sockfd, dns_query, query_ptr - dns_query, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bytes_sent < 0) {
        perror("Failed to send DNS query");
        exit(EXIT_FAILURE);
    }

    // Receive the DNS response
    ssize_t bytes_received;
    unsigned char response[512];
    memset(response, 0, sizeof(response));
    bytes_received = recvfrom(sockfd, response, sizeof(response), 0, NULL, NULL);
    if (bytes_received < 0) {
        perror("Failed to receive DNS response");
        exit(EXIT_FAILURE);
    }

    // Process the DNS response (this is just a placeholder for the actual processing)
    printf("Received %zd bytes of DNS response\n", bytes_received);

    // Close the socket
    close(sockfd);

    return 0;
}
