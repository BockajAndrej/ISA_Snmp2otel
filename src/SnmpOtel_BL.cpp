//
// Created by andrej.bockaj on 14. 10. 2025.
// login: xbockaa00
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
    // Registracia ctrl+c pre ukoncenie programu
    SignalHandler monitor;
    monitor.register_handler();

    SnmpOtelBer snmpBer = SnmpOtelBer(1);
    SnmpOtelExport snmpExport = SnmpOtelExport();
    SnmpOtelClient snmpClient = SnmpOtelClient();
    SnmpOtelJson snmpJsonCreator = SnmpOtelJson();
    SnmpOtelMetrics snmpMetrics = SnmpOtelMetrics();

    // Nacitanie oids zo suboru (moze definovat aj rozsirujuce informacie z formatmapping suboru)
    std::map<std::string, SnmpOtelOidConfig> rawMetrics = snmpMetrics.loadOtelMetrics(config.oids_file, config.oids_formatMappingFile);

    // Pre jednoduhsiu pracu len s oids
    std::vector<std::string> oids;
    for (auto &pair : rawMetrics)
    {
        oids.push_back(std::move(pair.first));
    }

    OpenTelemetryMetrics otelData;

    // Vytvorenie SNMP paketu pre posielanie cez UDP
    std::vector<unsigned char> msg = snmpBer.CreateSnmpMsg(config.community, 1, &oids);
    SNMP::StdByteVector receivedMsg;

    struct timeval tv{config.timeout / 1000, (config.timeout % 1000) * 1000};

    int ActualCountOfRepeats = 0;
    auto LastSendMessageTime = std::chrono::steady_clock::now() - std::chrono::seconds(config.interval);

    // Program funguje kym uzivatel neukonci (ctrl+c)
    while (SignalHandler::signal_received == 0)
    {
        // Atualizacia (update dat z snmpAgenta a nasledny export)
        if ((std::chrono::steady_clock::now() - LastSendMessageTime) >= std::chrono::seconds(config.interval))
        {
            // Ak odpoved z snmpAgenta nepride tak to skusi obmedzeni pocet krat poslat znova
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
                                  << "OIDS \n";

                        for (std::string oid : oids)
                        {
                            if (oid == oids[oids.size() - 1])
                                std::cout << "└── OID: " << oid << "\n";

                            else
                                std::cout << "├── OID: " << oid << "\n";
                        }
                        std::cout << "---------------" << std::endl;
                    }
                    // Aktualizacia hodnot pre OTEL (poslanie a prijatie SNMP spravy)
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
                        // V pripade ze aktualizacia bola uspesna tak dekodujeme
                        snmpBer.DecodeData(otelData, rawMetrics, receivedMsg, config);
                        ActualCountOfRepeats = 0;

                        // Timestamp
                        auto now = std::chrono::system_clock::now();
                        auto nano_time = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();

                        // Vytvorenie json spravy pre OTEL export
                        std::string jsonToSend = snmpJsonCreator.CreateOtlpMetricsJson(otelData);

                        if (config.verbose)
                        {
                            std::cout << "--- EXPORT ---\n"
                                      << "Target: " << config.endpoint << "\n"
                                      << "json:    " << jsonToSend << "\n"
                                      << "---------------" << std::endl;
                        }

                        // HTTP poslanie OTEL spravy
                        snmpExport.SendHttpMessage(config.endpoint, jsonToSend);
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
        }
        if ((std::chrono::seconds(config.interval) - (std::chrono::steady_clock::now() - LastSendMessageTime)) > std::chrono::milliseconds(100))
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return EXIT_SUCCESS;
}