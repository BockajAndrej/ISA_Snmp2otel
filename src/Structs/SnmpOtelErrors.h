//
// Created by andrej.bockaj on 14. 10. 2025.
// login: xbockaa00
//


#include <iostream>

#ifndef SNMP2OTEL_SNMPOTELERRORS_H
#define SNMP2OTEL_SNMPOTELERRORS_H

//Struktura pre definovanie vynimky pri nezadani vstupneho parametra
struct MissingArgumentError : public std::runtime_error {
    MissingArgumentError(const std::string& arg)
            : std::runtime_error("Mandatory argument is missing: " + arg) {}
};
//Struktura pre definovanie vynimky pri zadani nevalidnej hodnoty
struct InvalidValueError : public std::runtime_error {
    InvalidValueError(const std::string& arg, const std::string& val)
            : std::runtime_error("Invalid value for argument " + arg + ": " + val) {}
};


#endif //SNMP2OTEL_SNMPOTELERRORS_H
