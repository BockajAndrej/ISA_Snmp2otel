//
// Created by andrej.bockaj on 14. 10. 2025.
//

#include <iostream>
#include <fstream>

#include <cstring>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "SnmpOtel_BL.h"
#include "SnmpOtelBer.h"

SnmpOtel_BL::SnmpOtel_BL() {
    MAXLINE = 1024;
//        FLAGS = MSG_CONFIRM;
    FLAGS = 0;
}

int SnmpOtel_BL::MainFnc(const SnmpOtelConfig &config) {
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr;

    std::vector<std::string> oids;
    ReadOidsFromFile(config.oids_file, oids);

    auto* tmp2 = new SnmpOtelBer(1);
    std::vector<unsigned char> msg = tmp2->CreateSnmpMessage(config.community, 1, &oids);

    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    //Clear data
    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;                     //IPv4
    servaddr.sin_port = htons(config.port);   //Big-Endian setup
    servaddr.sin_addr.s_addr = INADDR_ANY;             //Each IP address which (local)pc has

    int n;
    socklen_t len;

    sendto(sockfd, msg.data(), msg.size(),
           FLAGS, (const struct sockaddr *) &servaddr,
           sizeof(servaddr));
    std::cout<<"Hello message sent."<<std::endl;

    n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                 MSG_WAITALL, (struct sockaddr *) &servaddr,
                 &len);
    buffer[n] = '\0';
    std::cout<<"Server :"<<buffer<<std::endl;

    close(sockfd);
    free(tmp2);
    return 0;
}

void SnmpOtel_BL::ReadOidsFromFile(const std::string &filename, std::vector<std::string> &oids) {
    std::ifstream inputFile(filename);

    if (!inputFile.is_open()) {
        std::cerr << "Internal Error: can not open file " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        line = TrimStringOid(line);
        if(!line.empty() && line.front() != '#'){
            oids.push_back(line);
        }
    }

    inputFile.close();
}

// Remove white chars from string
std::string SnmpOtel_BL::TrimStringOid(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    if (std::string::npos == start) {
        return "";
    }
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, (end - start + 1));
}


