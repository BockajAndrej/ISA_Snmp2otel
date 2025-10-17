//
// Created by andrej.bockaj on 17. 10. 2025.
//

#ifndef SNMP2OTEL_SNMPOTELEXPORTCONFIG_H
#define SNMP2OTEL_SNMPOTELEXPORTCONFIG_H

#include <string>

#include "../libs/nlohmann/json.hpp"

using json = nlohmann::json;

struct SnmpOtelExportConfig {
    std::string service_name = "ServerName";
    std::string metric_name = "MetricName";
    double metric_value = 0.0;
    std::string unit = "Unit";
    std::map<std::string, std::string> attributes;
};
#endif //SNMP2OTEL_SNMPOTELEXPORTCONFIG_H
