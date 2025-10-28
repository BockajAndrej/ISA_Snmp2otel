#include "../../libs/acutest/acutest.h"

#include <iostream>
#include <fstream>
#include <string>
#include <iterator>

#include "../../src/Structs/OpenTelemetryMetrics.h"
#include "../../src/SnmpOtelJson.h"

std::string nacitaj_cely_subor(const std::string &nazov_suboru)
{
    std::ifstream subor(nazov_suboru);

    if (!subor.is_open())
    {
        std::cerr << "Chyba: Nepodarilo sa otvoriť súbor " << nazov_suboru << std::endl;
        return "";
    }

    std::string obsah((std::istreambuf_iterator<char>(subor)),
                      std::istreambuf_iterator<char>());

    return obsah;
}
void odstran_medzery_a_nove_riadky(std::string &s)
{
    s.erase(
        std::remove_if(s.begin(), s.end(),
                       [](char c)
                       {
                           return (c == ' ' || c == '\n' || c == '\r');
                       }),
        s.end());
}

void otel_json_1(void)
{
    OpenTelemetryMetrics otelData;

    // --- Resource Metrics ---
    ResourceMetrics rm;

    // 1. Resource Attributes
    rm.resource.attributes.push_back({"service.name", "snmp2otel"});
    rm.resource.attributes.push_back({"host.name", "my-test-host"});

    // --- Scope Metrics ---
    ScopeMetrics sm;
    sm.scope.name = "snmp2otel.exporter";
    sm.scope.version = "1.0.0";

    // 2. Gauge Metric
    GaugeMetric gm;
    gm.name = "cpu_utilization";
    gm.description = "Aktuálne vyťaženie CPU";
    gm.unit = "%";
    gm.type = "gauge"; // Hodnota, ktorú kód očakáva pre uzol metriky

    // Data Point 1 (s časom a hodnotou)
    GaugeDataPoint dp1;
    dp1.timeUnixNano = "1700000000000000000";
    dp1.value = 45.5;

    // Data Point 2 (s časom, hodnotou A start_time_unix_nano)
    GaugeDataPoint dp2;
    dp2.timeUnixNano = "1700000001000000000";
    dp2.startTimeUnixNano = "1700000000000000000";
    dp2.value = 55.0;

    gm.dataPoints.push_back(dp1);
    gm.dataPoints.push_back(dp2);

    // Zostavenie hierarchie
    sm.metrics.push_back(gm);
    rm.scopeMetrics.push_back(sm);
    otelData.resourceMetrics.push_back(rm);

    SnmpOtelJson snmpJsonCreator = SnmpOtelJson();

    std::string obsah = nacitaj_cely_subor("tests/UnitTests/References/otelJsonUnitTests_1.json");

    // Act
    std::string result_json = snmpJsonCreator.CreateOtlpMetricsJson(otelData);

    odstran_medzery_a_nove_riadky(obsah);
    odstran_medzery_a_nove_riadky(result_json);

    // Assert
    TEST_CHECK(result_json == obsah);
}

TEST_LIST = {
    {"Test otel json create funcion", otel_json_1},
    {NULL, NULL} // END
};