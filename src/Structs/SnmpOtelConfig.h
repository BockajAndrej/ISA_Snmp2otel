//
// Created by andrej.bockaj on 14. 10. 2025.
// login: xbockaa00
//

#include <string>
#include <vector>

#ifndef SNMP2OTEL_SNMPOTELCONFIG_H
#define SNMP2OTEL_SNMPOTELCONFIG_H

// Struktura pre ulozenie vstupnych parametrov

struct SnmpOtelConfig
{
    std::string target;    // -t
    std::string oids_file; // -o
    std::string endpoint;  // -e

    std::string community = "private"; // -C
    int interval = 10;                 // -i
    int retries = 2;                   // -r
    int timeout = 1000;                // -T
    int port = 161;                    // -p
    bool verbose = false;              // -v

    std::string oids_formatMappingFile = ""; // -O

    bool help_requested = false; // Internal flag for --help / -h
};

#endif // SNMP2OTEL_SNMPOTELCONFIG_H
