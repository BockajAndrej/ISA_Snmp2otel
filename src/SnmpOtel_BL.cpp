//
// Created by andrej.bockaj on 14. 10. 2025.
//

#include <iostream>
#include <fstream>

#include <chrono>
#include <thread>

#include "SnmpOtel_BL.h"
#include "SnmpOtelBer.h"
#include "SnmpOtelExport.h"

#include "Handlers/SignalHandler.h"
#include "SnmpOtelClient.h"
#include "SnmpOtelJson.h"

SnmpOtel_BL::SnmpOtel_BL() {
    MAXLINE = 1024;
//        FLAGS = MSG_CONFIRM;
    FLAGS = 0;
}

int SnmpOtel_BL::MainFnc(const SnmpOtelConfig &config) {

    SignalHandler monitor;
    monitor.register_handler();

    std::vector<std::string> oids;
    ReadOidsFromFile(config.oids_file, oids);

    auto* snmpBer = new SnmpOtelBer(1);
    SnmpOtelExport snmpExport = SnmpOtelExport();
    SnmpOtelClient snmpClient = SnmpOtelClient();
    SnmpOtelJson snmpJsonCreator = SnmpOtelJson();

    std::vector<unsigned char> msg = snmpBer->CreateSnmpMsg(config.community, 1, &oids);
    SNMP::StdByteVector receivedMsg;


    struct timeval tv {config.timeout / 1000, (config.timeout % 1000) * 1000};

    int ActualCountOfRepeats = 0;
    auto LastSendMessageTime = std::chrono::steady_clock::now() - std::chrono::seconds(config.interval);

    while(SignalHandler::signal_received == 0){
        if((std::chrono::steady_clock::now() - LastSendMessageTime) >= std::chrono::seconds(config.interval)) {
            SnmpOtelExportConfig exportConfig;
            while (ActualCountOfRepeats++ < config.retries && SignalHandler::signal_received == 0) {

                LastSendMessageTime = std::chrono::steady_clock::now();
                try{
                    int status = snmpClient.UpdateData(receivedMsg, config.target, std::to_string(config.port),
                                                                         msg, tv);
                    if (status == EXIT_SUCCESS) {
                        snmpBer->DecodeData(exportConfig, receivedMsg);
                        ActualCountOfRepeats = 0;
                        break;
                    } else if (ActualCountOfRepeats == config.retries - 1) {
                        throw std::runtime_error("Unable to receive message from snmp agent");
                    }
                }catch(const std::runtime_error& e){
                    free(snmpBer);
                    throw std::runtime_error(e.what());
                }
            }
            std::string jsonToSend = snmpJsonCreator.CreateOtlpMetricsJson(exportConfig);
            std::cout << jsonToSend << std::endl;
            snmpExport.SendHttpMessage(config.endpoint, jsonToSend);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    free(snmpBer);
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