#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Create a UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    // Set up the server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on any available interface

    // Bind the socket to the server address
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return 1;
    }

    std::cout << "UDP server is listening on port " << SERVER_PORT << std::endl;

    while (true) {
        // Receive data from the client
        ssize_t recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (recv_len < 0) {
            perror("recvfrom");
            close(sockfd);
            return 1;
        }

        // Null-terminate the buffer
        buffer[recv_len] = '\0';

        // Print the received message
        std::cout << "{Received message from " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << "\t";
        std::cout << "Message: " << buffer << "}" << std::endl;
    }
    
    // Close the socket
    return close(sockfd);
}