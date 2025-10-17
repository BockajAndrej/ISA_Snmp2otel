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

#include <string>
#include <netdb.h>

#include <chrono>
#include <thread>

#include "SnmpOtel_BL.h"
#include "SnmpOtelBer.h"
#include "SnmpOtelExport.h"

#include "Handlers/SignalHandler.h"

SnmpOtel_BL::SnmpOtel_BL() {
    MAXLINE = 1024;
//        FLAGS = MSG_CONFIRM;
    FLAGS = 0;
}

int SnmpOtel_BL::MainFnc(const SnmpOtelConfig &config) {

    SignalHandler monitor;
    monitor.register_handler();

    struct sockaddr_in servaddr;

    std::vector<std::string> oids;
    ReadOidsFromFile(config.oids_file, oids);

    auto* berCreator = new SnmpOtelBer(1);
//    std::vector<unsigned char> msg = berCreator->CreateSnmpMessage(config.community, 1, &oids);

//    SnmpOtelExport tmp = SnmpOtelExport();
//    std::string str = tmp.create_json("Ahoj");
//
//    tmp.SendHttpMessage("www.vut.cz",str);
//    std::string str(msg.begin(), msg.end());

    std::vector<unsigned char> msg = berCreator->procesuj_paket(config.community, 1, &oids);

    struct timeval tv;
    tv.tv_sec = config.timeout / 1000;
    tv.tv_usec = (config.timeout % 1000) * 1000;

    int ActualCountOfRepeats = 0;

    auto LastSendMessageTime = std::chrono::steady_clock::now() - std::chrono::seconds(config.interval);

    while(SignalHandler::signal_received == 0){
        if((std::chrono::steady_clock::now() - LastSendMessageTime) >= std::chrono::seconds(config.interval)) {
            while (ActualCountOfRepeats++ < config.retries && SignalHandler::signal_received == 0) {

                LastSendMessageTime = std::chrono::steady_clock::now();
                try{
                    std::vector<char>* result = SendandReceiveUdpMessage(config.target, std::to_string(config.port),
                                                                         msg, tv);
                    if (result != nullptr) {
                        ActualCountOfRepeats = 0;
                        break;
                    } else if (ActualCountOfRepeats == config.retries - 1) {
                        throw std::runtime_error("Unable to receive message from snmp agent");
                    }
                }catch(const std::runtime_error& e){
                    free(berCreator);
                    throw std::runtime_error(e.what());
                }
            }
//            SendandReceiveTcpMessage()
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    free(berCreator);
    return EXIT_SUCCESS;
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

std::vector<char>* SnmpOtel_BL::SendandReceiveUdpMessage(const std::string &target_ip, const std::string &target_port, const std::vector<unsigned char>& message, struct timeval &tv) {
    struct addrinfo hints, *server_info, *p;
    int sockfd = EOF;
    int status;

    std::vector<char> receivedMsg;

    std::memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((status = getaddrinfo(target_ip.c_str(), target_port.c_str(), &hints, &server_info)) != 0) {
        throw std::runtime_error("ERROR at getaddrinfo for '" + target_ip + "': " + gai_strerror(status));
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
        return nullptr;
    }else {
        int n;
        socklen_t len = sizeof(*p);
        char buffer[MAXLINE];

        n = recvfrom(sockfd,
                     (char *) buffer,
                     sizeof(buffer) - 1,
                     0,
                     (struct sockaddr *) &p,
                     &len);
        if (n == EOF) {
            close(sockfd);
            freeaddrinfo(server_info);
            return nullptr;
        }
        for (int i = 0; i < n; ++i) {
            receivedMsg.push_back(buffer[i]);
        }
    }

    close(sockfd);
    freeaddrinfo(server_info);
    return &receivedMsg;
}