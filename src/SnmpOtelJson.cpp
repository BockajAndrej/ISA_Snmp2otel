//
// Created by andrej.bockaj on 17. 10. 2025.
//
#include "SnmpOtelJson.h"

SnmpOtelJson::SnmpOtelJson() = default;

std::string SnmpOtelJson::CreateOtlpMetricsJson(struct SnmpOtelExportConfig& config) {
    // Timestamp
    auto now = std::chrono::system_clock::now();
    auto nano_time = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    std::string time_unix_nano = std::to_string(nano_time);

    // Resource Attributes
    json resource_attributes = json::array();

    resource_attributes.push_back({
                                          {"key", "service.name"},
                                          {"value", {{"string_value", config.service_name}}}
                                  });

    // Data
    json data_point_attributes = json::array();
    for (const auto& [key, value] : config.attributes) {
        data_point_attributes.push_back({
                                                {"key", key},
                                                {"value", {{"string_value", value}}}
                                        });
    }

    json data_point = {
            {"time_unix_nano", time_unix_nano},
            {"as_double", config.metric_value},
            {"attributes", data_point_attributes}
    };

    // Final JSON
    json metrics_json = {
            {"resource_metrics", {
                    {
                            // Resource
                            {"resource", {
                                    {"attributes", resource_attributes}
                            }},
                            // Scope Metrics
                            {"scope_metrics", {
                                    {
                                            {"scope", {{"name", "snmp2otel.exporter"}}},
                                            {"metrics", {
                                                    {
                                                            {"name", config.metric_name},
                                                            {"unit", config.unit},
                                                            {"description", "Metric exported from SNMP device"},
                                                            // Gauge Metrics
                                                            {"gauge", {
                                                                    {"data_points", {data_point}}
                                                            }}
                                                    }
                                            }}
                                    }
                            }}
                    }
            }}
    };
    return metrics_json.dump();
}
