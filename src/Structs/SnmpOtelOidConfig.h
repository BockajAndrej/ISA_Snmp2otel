//
// Created by andrej.bockaj on 17. 10. 2025.
// login: xbockaa00
//

#ifndef SNMP2OTEL_SNMPOTELOIDCONFIG_H
#define SNMP2OTEL_SNMPOTELOIDCONFIG_H

#include <string>
#include <map>

//Struktura definujuca oid s volitelnymi atributmi z format mapping oid suboru

struct SnmpOtelOidConfig
{
    std::string oid;
    std::string name = "undefined";
    std::string unit = "1";
    std::string type = "gauge";

    SnmpOtelOidConfig(std::string o) 
        : oid(std::move(o)), name("snmp." + oid) {}
};

#endif //SNMP2OTEL_SNMPOTELOIDCONFIG_H