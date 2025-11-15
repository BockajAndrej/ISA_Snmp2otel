#include "SnmpOtelMetrics.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>

#include "../libs/nlohmann/json.hpp"

SnmpOtelMetrics::SnmpOtelMetrics()
{
}

std::string SnmpOtelMetrics::trim(const std::string &str)
{
    const std::string whitespace = " \t\n\r\f\v";
    size_t first = str.find_first_not_of(whitespace);
    if (std::string::npos == first)
    {
        return str;
    }
    size_t last = str.find_last_not_of(whitespace);
    return str.substr(first, (last - first + 1));
}

std::map<std::string, SnmpOtelOidConfig> SnmpOtelMetrics::readOidsFile(const std::string &filename)
{
    std::map<std::string, SnmpOtelOidConfig> metrics;
    std::ifstream file(filename);

    if (!file.is_open())
    {
        throw std::runtime_error("Unable to open mandatory oids file");
    }

    std::string line;
    while (std::getline(file, line))
    {
        line = trim(line);

        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        metrics.emplace(line, SnmpOtelOidConfig(line));
    }

    return metrics;
}

std::map<std::string, SnmpOtelOidConfig> SnmpOtelMetrics::readFormatOidsFile(const std::string &filename)
{
    std::map<std::string, SnmpOtelOidConfig> formatMetrics;
    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "INFO: Nepodarilo sa otvorit subor s formatmi: " << filename << ". Budu pouzite len default hodnoty." << std::endl;
        return formatMetrics;
    }

    try
    {
        nlohmann::json j;
        file >> j;

        if (!j.is_object())
        {
            std::cerr << "CHYBA: Format suboru nie je validny JSON objekt." << std::endl;
            return formatMetrics;
        }

        // Iterujeme cez kluce
        for (auto const &[oid_key, oid_value] : j.items())
        {
            if (oid_value.is_object())
            {
                SnmpOtelOidConfig metric(oid_key);

                // Vyplnenie hodnôt z JSON (s kontrolou existencie)
                if (oid_value.contains("name") && oid_value["name"].is_string())
                {
                    metric.name = oid_value["name"].get<std::string>();
                }
                if (oid_value.contains("unit") && oid_value["unit"].is_string())
                {
                    metric.unit = oid_value["unit"].get<std::string>();
                }
                if (oid_value.contains("type") && oid_value["type"].is_string())
                {
                    metric.type = oid_value["type"].get<std::string>();
                }

                formatMetrics.emplace(oid_key, std::move(metric));
            }
        }
    }
    catch (const nlohmann::json::parse_error &e)
    {
        std::cerr << "CHYBA: Chyba pri parsingu JSON suboru " << filename << ": " << e.what() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "CHYBA: Neznama chyba pri spracovani JSON: " << e.what() << std::endl;
    }

    return formatMetrics;
}

std::map<std::string, SnmpOtelOidConfig> SnmpOtelMetrics::loadOtelMetrics(const std::string &oidsFile, const std::string &formatFile)
{
    std::map<std::string, SnmpOtelOidConfig> metrics = readOidsFile(oidsFile);
    std::map<std::string, SnmpOtelOidConfig> formatMetrics = readFormatOidsFile(formatFile);

    // Aplikacia definicie na nacitanie OID
    for (auto &pair : metrics)
    {
        const std::string &oid = pair.first;
        SnmpOtelOidConfig &metric = pair.second;

        if (formatMetrics.count(oid))
        {
            const SnmpOtelOidConfig &format = formatMetrics.at(oid);

            metric.name = format.name;
            metric.unit = format.unit;
            metric.type = format.type;
        }
    }

    return metrics;
}