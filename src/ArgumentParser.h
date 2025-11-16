//
// Created by andrej.bockaj on 14. 10. 2025.
// login: xbockaa00
//


#include "Structs/SnmpOtelConfig.h"

#ifndef SNMP2OTEL_ARGUMENTPARSER_H
#define SNMP2OTEL_ARGUMENTPARSER_H

class ArgumentParser {
public:
    SnmpOtelConfig Parse(int argc, char* argv[]);
    void PrintHelp(const std::string& app_name) const;

private:
    SnmpOtelConfig config_;

    std::string GetNextValue(const std::vector<std::string>& args, size_t& index, const std::string& arg_name);
    int ParseIntValue(const std::vector<std::string>& args, size_t& index, const std::string& arg_name);
    void CheckRequired(const std::string& name, const std::string& value);
};


#endif //SNMP2OTEL_ARGUMENTPARSER_H
