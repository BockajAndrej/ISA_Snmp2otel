//
// Created by andrej.bockaj on 14. 10. 2025.
//

#ifndef SNMP2OTEL_SNMPOTELBER_H
#define SNMP2OTEL_SNMPOTELBER_H

#include <string>
#include <vector>

#include "../libs/BasicSnmp/lib/snmplib.h"

#include "Structs/OpenTelemetryMetrics.h"
#include "Structs/SnmpOtelOidConfig.h"
#include "Structs/SnmpOtelConfig.h"

class SnmpOtelBer {
public:
    explicit SnmpOtelBer(int SnmpVersion);
    SNMP::StdByteVector CreateSnmpMsg(const std::string& CommunityString, int PduType, std::vector<std::string> *OidsRaw);
    int DecodeData(OpenTelemetryMetrics &otelMetrics, std::map<std::string, SnmpOtelOidConfig> &oidMetrics, SNMP::StdByteVector bytes, const SnmpOtelConfig &config);

private:
    char SnmpVersion;
    std::string getCurrentTimeUnixNano();
    std::string removeLeadingDot(const std::string& oid);
};


#endif //SNMP2OTEL_SNMPOTELBER_H
