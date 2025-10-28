//
// Created by andrej.bockaj on 17. 10. 2025.
//

#ifndef SNMP2OTEL_SNMPOTELJSON_H
#define SNMP2OTEL_SNMPOTELJSON_H

#include "../libs/nlohmann/json.hpp"
#include "Structs/OpenTelemetryMetrics.h"

using json = nlohmann::json;

class SnmpOtelJson {
public:
    explicit SnmpOtelJson();
    std::string CreateOtlpMetricsJson(const OpenTelemetryMetrics& otelMetrics);
private:

};


#endif //SNMP2OTEL_SNMPOTELJSON_H
