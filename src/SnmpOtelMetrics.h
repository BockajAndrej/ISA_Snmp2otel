//
// Created by andrej.bockaj on 17. 10. 2025.
// login: xbockaa00
//


#ifndef SNMP2OTEL_SNMPOTELMETRICS_H
#define SNMP2OTEL_SNMPOTELMETRICS_H

#include "vector"
#include "Structs/SnmpOtelOidConfig.h"

class SnmpOtelMetrics {
public:
    SnmpOtelMetrics();
    std::map<std::string, SnmpOtelOidConfig> loadOtelMetrics(const std::string& oidsFile, const std::string& formatFile);

private:
    std::string trim(const std::string& str);
    std::map<std::string, SnmpOtelOidConfig> readOidsFile(const std::string& filename);
    std::map<std::string, SnmpOtelOidConfig> readFormatOidsFile(const std::string& filename);
};


#endif //SNMP2OTEL_SNMPOTELMETRICS_H