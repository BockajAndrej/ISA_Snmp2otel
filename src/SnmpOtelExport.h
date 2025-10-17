//
// Created by andrej.bockaj on 15. 10. 2025.
//

#ifndef SNMP2OTEL_SNMPOTELEXPORT_H
#define SNMP2OTEL_SNMPOTELEXPORT_H

#include <string>

class SnmpOtelExport {
public:
    std::string create_json(std::string msg);
    int SendHttpMessage(const std::string& target_ip, const std::string& message);


private:

};


#endif //SNMP2OTEL_SNMPOTELEXPORT_H
