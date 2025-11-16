//
// Created by andrej.bockaj on 17. 10. 2025.
// login: xbockaa00
//

#include "SnmpOtelJson.h"

SnmpOtelJson::SnmpOtelJson() = default;

// Funkcia na prevod OTEL struktury na string s formatom json 
std::string SnmpOtelJson::CreateOtlpMetricsJson(const OpenTelemetryMetrics &otelMetrics)
{
    json root_json;
    json resource_metrics_array = json::array();

    for (const auto& resourceMetrics : otelMetrics.resourceMetrics) {
        json resource_attributes_array = json::array();
        
        // Resource Attributes
        for (const auto& attr : resourceMetrics.resource.attributes) {
            resource_attributes_array.push_back({
                {"key", attr.key},
                {"value", {{"string_value", attr.value}}}
            });
        }

        json scope_metrics_array = json::array();
        for (const auto& scopeMetrics : resourceMetrics.scopeMetrics) {
            
            json metrics_array = json::array();
            for (const auto& metric : scopeMetrics.metrics) {
                
                json data_points_array = json::array();
                for (const auto& dataPoint : metric.dataPoints) {
                    json dp_attributes_array = json::array();
                    
                    // Data Point
                    json data_point_json = {
                        {"time_unix_nano", dataPoint.timeUnixNano},
                        {"as_double", dataPoint.value}
                    };
                    
                    // Len ak je startTimeUnixNano neprázdne, pridá sa
                    if (!dataPoint.startTimeUnixNano.empty()) {
                        data_point_json["start_time_unix_nano"] = dataPoint.startTimeUnixNano;
                    }
                    
                    data_points_array.push_back(data_point_json);
                }

                // Metric (Gauge)
                metrics_array.push_back({
                    {"name", metric.name},
                    {"description", metric.description},
                    {"unit", metric.unit},
                    {metric.type, {
                        {"data_points", data_points_array}
                    }}
                });
            }

            // Scope Metrics
            scope_metrics_array.push_back({
                {"scope", {
                    {"name", scopeMetrics.scope.name},
                    {"version", scopeMetrics.scope.version}
                }},
                {"metrics", metrics_array}
            });
        }

        // Resource Metrics
        resource_metrics_array.push_back({
            {"resource", {
                {"attributes", resource_attributes_array}
            }},
            {"scopeMetrics", scope_metrics_array}
        });
    }

    // Final JSON
    root_json["resourceMetrics"] = resource_metrics_array;

    return root_json.dump();
}