//
// Created by andrej.bockaj on 14. 10. 2025.
// login: xbockaa00
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>

#include "SnmpOtelBer.h"

SnmpOtelBer::SnmpOtelBer(int SnmpVersion)
{
    // Len definujeme verziu ktora by sa nemala menit pri rovnakej instancii SNMP creatora
    this->SnmpVersion = SnmpVersion;
}

// Funkcia pre naplnenie hodnot SNMP paketu
SNMP::StdByteVector SnmpOtelBer::CreateSnmpMsg(const std::string &CommunityString, int PduType, std::vector<std::string> *OidsRaw)
{

    SNMP::OIDList normalizedOids;
    for (const std::string &strOid : *OidsRaw)
    {
        normalizedOids.push_back(SNMP::OID(strOid));
    }

    SNMP::Encoder snmp;
    snmp.setupGetRequest(SnmpVersion, CommunityString, PduType, normalizedOids);
    return snmp.encodeRequest();
}
// Funkcia plniaca OTEL strukturu z raw SNMP dat (riesi dokodovanie)
int SnmpOtelBer::DecodeData(OpenTelemetryMetrics &otelMetrics, std::map<std::string, SnmpOtelOidConfig> &oidMetrics, SNMP::StdByteVector bytes, const SnmpOtelConfig &config)
{
    SNMP::Encoder snmp;
    if (!snmp.decodeAll(bytes, true)) // true - to keep raw data (useful for debugging)
    {
        throw std::runtime_error(snmp.errorCode() + " in object " + snmp.errorObjectIndex());
        return EXIT_FAILURE;
    }

    std::string timeUnixNano = getCurrentTimeUnixNano();

    ResourceMetrics rm;
    rm.resource.attributes.push_back({"service.name", "snmp2otel"});
    rm.resource.attributes.push_back({"host.name", config.target});

    ScopeMetrics sm;
    sm.scope.name = "snmp2otel.exporter";
    sm.scope.version = "1.0.0";

    // Spracuje vsetky OIDS ktore snmpAgent zaslal
    for (const auto &asn1Varbind : snmp.varbindList())
    {
        std::string metricName = asn1Varbind.oid().toStdString();
        bool isNumeric = false;
        double value = 0.0;
        std::string valueStr;

        // Ukladanie podla typu (Riesime hlavne Gauge32)
        switch (asn1Varbind.asn1Variable().type())
        {
        case ASN1TYPE_NULL:
            break;
        case ASN1TYPE_OCTETSTRING:
        case ASN1TYPE_IA5String:
        case ASN1TYPE_VideoString:
            valueStr = asn1Varbind.asn1Variable().toStdString();
            break;
        case ASN1TYPE_INTEGER:
        case ASN1TYPE_Integer64:
        case ASN1TYPE_Unsigned64:
        case ASN1TYPE_Counter64:
        case ASN1TYPE_Gauge32:
            isNumeric = true;
            value = static_cast<double>(asn1Varbind.asn1Variable().toInteger64());
            break;
        default:
            break;
        }

        // Plnenie datovej casti OTEL struktury

        if (!otelMetrics.resourceMetrics.empty())
        {
            otelMetrics.resourceMetrics.pop_back();
        }

        if (isNumeric)
        {

            auto result = oidMetrics.find(removeLeadingDot(metricName));
            if (result != oidMetrics.end())
            {
                const SnmpOtelOidConfig &config = result->second;

                GaugeMetric gm;

                gm.name = config.name;
                gm.unit = config.unit;
                gm.type = config.type;

                GaugeDataPoint dp;
                dp.startTimeUnixNano = timeUnixNano;
                dp.timeUnixNano = timeUnixNano;
                dp.value = value;

                gm.dataPoints.push_back(dp);
                sm.metrics.push_back(gm);
            }
        }
    }

    rm.scopeMetrics.push_back(sm);
    otelMetrics.resourceMetrics.push_back(rm);

    return EXIT_SUCCESS;
}

std::string SnmpOtelBer::getCurrentTimeUnixNano()
{
    using namespace std::chrono;
    auto now = high_resolution_clock::now();
    auto nano_since_epoch = duration_cast<nanoseconds>(now.time_since_epoch()).count();
    return std::to_string(nano_since_epoch);
}

// Funkcia odstranujuca bodku pred OID (po dekodovani kniznica BasicSnmp pridava bodku pred OID)
std::string SnmpOtelBer::removeLeadingDot(const std::string &oid)
{
    if (!oid.empty() && oid[0] == '.')
    {
        return oid.substr(1);
    }
    return oid;
}