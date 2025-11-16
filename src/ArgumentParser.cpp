//
// Created by andrej.bockaj on 14. 10. 2025.
// login: xbockaa00
//

#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

#include "ArgumentParser.h"
#include "Structs/SnmpOtelErrors.h"

SnmpOtelConfig ArgumentParser::Parse(int argc, char **argv)
{
    auto config = SnmpOtelConfig{};

    std::vector<std::string> args(argv + 1, argv + argc);

    for (size_t i = 0; i < args.size(); ++i)
    {
        const std::string &arg = args[i];

        if (arg == "-t")
        {
            config.target = GetNextValue(args, i, "-t");
        }
        else if (arg == "-o")
        {
            config.oids_file = GetNextValue(args, i, "-o");
        }
        else if (arg == "-e")
        {
            config.endpoint = GetNextValue(args, i, "-e");
        }
        else if (arg == "-C")
        {
            config.community = GetNextValue(args, i, "-C");
        }
        else if (arg == "-i")
        {
            config.interval = ParseIntValue(args, i, "-i");
        }
        else if (arg == "-r")
        {
            config.retries = ParseIntValue(args, i, "-r");
        }
        else if (arg == "-T")
        {
            config.timeout = ParseIntValue(args, i, "-T");
        }
        else if (arg == "-p")
        {
            config.port = ParseIntValue(args, i, "-p");
        }

        else if (arg == "-v")
        {
            config.verbose = true;
        }
        else if (arg == "-h" || arg == "--help")
        {
            config.help_requested = true;
            return config;
        }
        else if (arg == "-O")
        {
            config.oids_formatMappingFile = GetNextValue(args, i, "-O");
        }
    }

    // Required
    CheckRequired("-t", config.target);
    CheckRequired("-o", config.oids_file);
    CheckRequired("-e", config.endpoint);

    return config;
}

void ArgumentParser::PrintHelp(const std::string &app_name) const
{
    std::cout << "Použitie: " << app_name << " -t target [-C community] -o oids_file -e endpoint ";
    std::cout << "[-i interval] [-r retries] [-T timeout] [-p port] [-v]\n\n";
    std::cout << "Volby:\n";
    std::cout << "  -t <target>       Mandatory: IP/Hostname cieľového zariadenia.\n";
    std::cout << "  -o <file>         Mandatory: Cesta k súboru s OID.\n";
    std::cout << "  -e <endpoint>     Mandatory: OTEL Collector endpoint.\n";
    std::cout << "  -C <community>    SNMP Community string (predvolené: " << config_.community << ").\n";
    std::cout << "  -i <interval>     Interval dotazovania v sekundách (predvolené: " << config_.interval << ").\n";
    std::cout << "  -r <retries>      Počet pokusov (predvolené: " << config_.retries << ").\n";
    std::cout << "  -T <timeout>      Timeout (ms) (predvolené: " << config_.timeout << ").\n";
    std::cout << "  -p <port>         SNMP port (predvolené: " << config_.port << ").\n";
    std::cout << "  -v                Zapne detailné logovanie (verbose).\n";
    std::cout << "  -v                Zapne detailné logovanie (verbose).\n";
    std::cout << "  -h, --help        Zobrazí túto správu.\n";
}

std::string
ArgumentParser::GetNextValue(const std::vector<std::string> &args, size_t &index, const std::string &arg_name)
{
    if (index + 1 >= args.size())
    {
        throw MissingArgumentError(arg_name + " (Value is missing)");
    }
    return args[++index];
}

int ArgumentParser::ParseIntValue(const std::vector<std::string> &args, size_t &index, const std::string &arg_name)
{
    std::string value_str = GetNextValue(args, index, arg_name);
    try
    {
        return std::stoi(value_str);
    }
    catch (const std::invalid_argument &)
    {
        throw InvalidValueError(arg_name, value_str + " (Invalid integer)");
    }
    catch (const std::out_of_range &)
    {
        throw InvalidValueError(arg_name, value_str + " (Number out of range)");
    }
}

void ArgumentParser::CheckRequired(const std::string &name, const std::string &value)
{
    if (value.empty())
    {
        throw MissingArgumentError(name);
    }
}
