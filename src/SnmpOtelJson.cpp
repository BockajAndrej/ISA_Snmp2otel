//
// Created by andrej.bockaj on 17. 10. 2025.
//
#include "SnmpOtelJson.h"

SnmpOtelJson::SnmpOtelJson() = default;

std::string SnmpOtelJson::CreateOtlpMetricsJson(const OpenTelemetryMetrics &otelMetrics)
{
    json root_json;
    json resource_metrics_array = json::array();

    for (const auto& resourceMetrics : otelMetrics.resourceMetrics) {
        json resource_attributes_array = json::array();
        
        // 1. Resource Attributes
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
                    
                    // 3. Data Point
                    json data_point_json = {
                        {"time_unix_nano", dataPoint.timeUnixNano},
                        {"as_double", dataPoint.value}
                    };
                    
                    // Len ak je startTimeUnixNano neprázdne, pridá sa (Gauge ho zvyčajne nemá)
                    if (!dataPoint.startTimeUnixNano.empty()) {
                        data_point_json["start_time_unix_nano"] = dataPoint.startTimeUnixNano;
                    }
                    
                    data_points_array.push_back(data_point_json);
                }

                // 4. Metric (Gauge)
                metrics_array.push_back({
                    {"name", metric.name},
                    {"description", metric.description},
                    {"unit", metric.unit},
                    {metric.type, {
                        {"data_points", data_points_array}
                    }}
                });
            }

            // 5. Scope Metrics
            scope_metrics_array.push_back({
                {"scope", {
                    {"name", scopeMetrics.scope.name},
                    {"version", scopeMetrics.scope.version}
                }},
                {"metrics", metrics_array}
            });
        }

        // 6. Resource Metrics
        resource_metrics_array.push_back({
            {"resource", {
                {"attributes", resource_attributes_array}
            }},
            {"scopeMetrics", scope_metrics_array}
        });
    }

    // 7. Final JSON
    root_json["resourceMetrics"] = resource_metrics_array;

    return root_json.dump();
}