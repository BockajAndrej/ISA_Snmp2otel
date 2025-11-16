//
// Created by andrej.bockaj on 17. 10. 2025.
// login: xbockaa00
//

#ifndef SNMP2OTEL_SNMPOTELEXPORTCONFIG_H
#define SNMP2OTEL_SNMPOTELEXPORTCONFIG_H

#include <string>

#include "../../libs/nlohmann/json.hpp"

using json = nlohmann::json;

//Struktura definujuca strukturu OTEL/HTTP/Json spravy pre OTEL endpoint

struct Attribute
{
    std::string key;
    std::string value;
};

struct GaugeDataPoint
{
    std::string startTimeUnixNano;
    std::string timeUnixNano;
    double value;
};

struct GaugeMetric
{
    std::string name;
    std::string description;
    std::string unit;
    std::string type;
    std::vector<GaugeDataPoint> dataPoints;
};

struct Scope
{
    std::string name;
    std::string version;
};

struct ScopeMetrics
{
    Scope scope;
    std::vector<GaugeMetric> metrics;
};

struct Resource
{
    std::vector<Attribute> attributes;
};

struct ResourceMetrics
{
    Resource resource;
    std::vector<ScopeMetrics> scopeMetrics;
};

struct OpenTelemetryMetrics
{
    std::vector<ResourceMetrics> resourceMetrics;
};
#endif //SNMP2OTEL_SNMPOTELEXPORTCONFIG_H
