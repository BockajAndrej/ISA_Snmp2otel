//
// Created by andrej.bockaj on 12. 10. 2025.
//
#include <iostream>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "snmpBer.h"
#include "Snmp2otel_BL.h"

namespace snmp2otel_bl {
    Snmp2otel_BL::Snmp2otel_BL() {
        PORT = 8080;
        MAXLINE = 1024;
//        FLAGS = MSG_CONFIRM;
        FLAGS = 0;
    }
    int Snmp2otel_BL::TmpFuc(){
        std::vector<std::string> oid;
        oid.push_back("1.3.6.1.4.1.2680.1.2.7.3.2.0");
        auto* tmp2 = new snmpber::snmpBer(1);
        std::vector<unsigned char> value = tmp2->CreateSnmpMessage("private", 1, &oid);
        std::cout << "\n Started characters: \n";
        for(unsigned char c : value){
            std::cout << std::hex << (int)c << " ";
        }
        std::cout << "\n";
        std::cout << std::dec << "OUT > Msg= " << ", Length= " << value.size();
        free(tmp2);
        return 0;
    }
    int Snmp2otel_BL::mainFnc() {
        int sockfd;
        char buffer[MAXLINE];
        struct sockaddr_in     servaddr;

        std::vector<std::string> oid;
        oid.push_back("1.3.6.1.4.1.2680.1.2.7.3.2.0");
        auto* tmp2 = new snmpber::snmpBer(1);
        std::vector<unsigned char> msg = tmp2->CreateSnmpMessage("private", 1, &oid);

        // Creating socket file descriptor
        if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }

        //Clear data
        memset(&servaddr, 0, sizeof(servaddr));

        // Filling server information
        servaddr.sin_family = AF_INET;              //IPv4
        servaddr.sin_port = htons(PORT);   //Big-Endian setup
        servaddr.sin_addr.s_addr = INADDR_ANY;      //Each IP address which (local)pc has

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
} // snmp2otel_bl