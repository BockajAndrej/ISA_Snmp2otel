//
// Created by andrej.bockaj on 14. 10. 2025.
//

#ifndef SNMP2OTEL_SNMPOTELBER_H
#define SNMP2OTEL_SNMPOTELBER_H

#include <string>
#include <vector>

#include "../libs/BasicSnmp/lib/snmplib.h"

#include "Structs/SnmpOtelExportConfig.h"

class SnmpOtelBer {
public:
    explicit SnmpOtelBer(int SnmpVersion);
    SNMP::StdByteVector CreateSnmpMsg(const std::string& CommunityString, int PduType, std::vector<std::string> *OidsRaw);
    int DecodeData(SnmpOtelExportConfig& config, SNMP::StdByteVector bytes);


private:
    int RequestID;
    int Error;
    int ErrorIndex;
    char SnmpVersion;
};


#endif //SNMP2OTEL_SNMPOTELBER_H
