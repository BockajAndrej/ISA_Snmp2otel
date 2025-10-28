//
// Created by andrej.bockaj on 17. 10. 2025.
//

#include <cstring>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>

#include <string>
#include <netdb.h>

#include "Handlers/SignalHandler.h"

#include "SnmpOtelClient.h"

int SnmpOtelClient::UpdateData(std::vector<unsigned char> &receivedMsg, const std::string &target_ip, const std::string &target_port,
                                         const std::vector<unsigned char> &message, timeval &tv) {
    struct addrinfo hints, *server_info, *p;
    int sockfd = EOF;
    int status;

    receivedMsg.clear();

    std::memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((status = getaddrinfo(target_ip.c_str(), target_port.c_str(), &hints, &server_info)) != 0) {
        throw std::runtime_error("Error at getaddrinfo for '" + target_ip + "': " + gai_strerror(status));
    }

    for(p = server_info; p != nullptr; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) != EOF) {
            setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
            break;
        }
        std::cerr << "ERROR: Socket for domain '" << p->ai_addr << "' : Can not be established";
    }

    if (p == nullptr) {
        freeaddrinfo(server_info);
        throw std::runtime_error("Nepodarilo sa vytvoriť socket pre žiadnu adresu.");
    }

    ssize_t bytes_sent = sendto(
            sockfd,
            message.data(),
            message.size(),
            0,
            p->ai_addr,
            p->ai_addrlen
    );

    if(bytes_sent == EOF){
        std::cerr << "ERROR: Message was unable to send";
        close(sockfd);
        freeaddrinfo(server_info);
        return EXIT_FAILURE;
    }else {
        int n;
        socklen_t len = sizeof(*p);
        char buffer[MAXLEN];

        n = recvfrom(sockfd,
                     (char *) buffer,
                     sizeof(buffer) - 1,
                     0,
                     (struct sockaddr *) &p,
                     &len);
        if (n == EOF) {
            close(sockfd);
            freeaddrinfo(server_info);
            return EXIT_FAILURE;
        }
        for (int i = 0; i < n; ++i) {
            receivedMsg.push_back(buffer[i]);
        }
    }

    close(sockfd);
    freeaddrinfo(server_info);
    return EXIT_SUCCESS;
}