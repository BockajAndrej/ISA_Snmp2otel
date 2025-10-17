//
// Created by andrej.bockaj on 14. 10. 2025.
//

#ifndef SNMP2OTEL_SNMPOTEL_BL_H
#define SNMP2OTEL_SNMPOTEL_BL_H

#include "Structs/SnmpOtelConfig.h"

class SnmpOtel_BL {
public:
    SnmpOtel_BL();
    int MainFnc(const SnmpOtelConfig& config);

private:
    int MAXLINE;
    int FLAGS;

    void ReadOidsFromFile(const std::string& filename, std::vector<std::string> &oids);
    std::string TrimStringOid(const std::string& s);

};


#endif //SNMP2OTEL_SNMPOTEL_BL_H
