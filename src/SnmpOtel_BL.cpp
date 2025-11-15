//
// Created by andrej.bockaj on 14. 10. 2025.
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>

#include <chrono>
#include <thread>

#include "SnmpOtel_BL.h"
#include "SnmpOtelBer.h"
#include "SnmpOtelExport.h"

#include "Handlers/SignalHandler.h"
#include "Structs/OpenTelemetryMetrics.h"
#include "Structs/SnmpOtelOidConfig.h"

#include "SnmpOtelClient.h"
#include "SnmpOtelJson.h"
#include "SnmpOtelMetrics.h"

SnmpOtel_BL::SnmpOtel_BL()
{
    MAXLINE = 1024;
    FLAGS = 0;
}

int SnmpOtel_BL::MainFnc(const SnmpOtelConfig &config)
{
    SignalHandler monitor;
    monitor.register_handler();

    SnmpOtelBer snmpBer = SnmpOtelBer(1);
    SnmpOtelExport snmpExport = SnmpOtelExport();
    SnmpOtelClient snmpClient = SnmpOtelClient();
    SnmpOtelJson snmpJsonCreator = SnmpOtelJson();
    SnmpOtelMetrics snmpMetrics = SnmpOtelMetrics();

    std::map<std::string, SnmpOtelOidConfig> rawMetrics = snmpMetrics.loadOtelMetrics(config.oids_file, "");

    std::vector<std::string> oids;
    for (auto &pair : rawMetrics)
    {
        oids.push_back(std::move(pair.first));
    }

    OpenTelemetryMetrics otelData;

    std::vector<unsigned char> msg = snmpBer.CreateSnmpMsg(config.community, 1, &oids);
    SNMP::StdByteVector receivedMsg;

    struct timeval tv{config.timeout / 1000, (config.timeout % 1000) * 1000};

    int ActualCountOfRepeats = 0;
    auto LastSendMessageTime = std::chrono::steady_clock::now() - std::chrono::seconds(config.interval);

    while (SignalHandler::signal_received == 0)
    {
        if ((std::chrono::steady_clock::now() - LastSendMessageTime) >= std::chrono::seconds(config.interval))
        {
            while (ActualCountOfRepeats++ < config.retries && SignalHandler::signal_received == 0)
            {

                LastSendMessageTime = std::chrono::steady_clock::now();
                try
                {
                    if (config.verbose)
                    {
                        std::cout << "--- REQUEST ---\n"
                                  << "Target: " << config.target << "\n"
                                  << "Port:   " << config.port << "\n"
                                  << "Msg:    " <<reinterpret_cast<const char*>(msg.data()) << "\n"
                                  << "---------------" << std::endl;
                    }

                    int status = snmpClient.UpdateData(receivedMsg, config.target, std::to_string(config.port),
                                                       msg, tv);
                    if (status == EXIT_SUCCESS)
                    {
                        if (config.verbose)
                        {
                            std::cout << "--- ANSWER ---\n"
                                      << "From: " << config.target << "\n"
                                      << "Port:   " << config.port << "\n"
                                      << "Recv msg:    " << receivedMsg.toStdString() << "\n"
                                      << "---------------" << std::endl;
                        }
                        snmpBer.DecodeData(otelData, rawMetrics, receivedMsg, config);
                        ActualCountOfRepeats = 0;
                        break;
                    }
                    else if (ActualCountOfRepeats == config.retries - 1)
                    {
                        throw std::runtime_error("Unable to receive message from snmp agent");
                    }
                }
                catch (const std::runtime_error &e)
                {
                    throw std::runtime_error(e.what());
                }
            }
            // Timestamp
            auto now = std::chrono::system_clock::now();
            auto nano_time = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();

            std::string jsonToSend = snmpJsonCreator.CreateOtlpMetricsJson(otelData);
            std::cout << jsonToSend << std::endl;

            if (config.verbose)
            {
                std::cout << "--- EXPORT ---\n"
                          << "Target: " << config.endpoint << "\n"
                          << "json:    " << jsonToSend << "\n"
                          << "---------------" << std::endl;
            }

            snmpExport.SendHttpMessage(config.endpoint, jsonToSend);
        }
        if ((std::chrono::seconds(config.interval) - (std::chrono::steady_clock::now() - LastSendMessageTime)) > std::chrono::milliseconds(100))
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return EXIT_SUCCESS;
}