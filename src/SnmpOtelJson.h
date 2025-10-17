//
// Created by andrej.bockaj on 17. 10. 2025.
//

#ifndef SNMP2OTEL_SNMPOTELJSON_H
#define SNMP2OTEL_SNMPOTELJSON_H

#include "../libs/nlohmann/json.hpp"
#include "Structs/SnmpOtelExportConfig.h"

using json = nlohmann::json;

class SnmpOtelJson {
public:
    explicit SnmpOtelJson();
    std::string CreateOtlpMetricsJson(struct SnmpOtelExportConfig& config);

private:

};


#endif //SNMP2OTEL_SNMPOTELJSON_H
