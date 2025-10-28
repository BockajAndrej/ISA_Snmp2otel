//
// Created by andrej.bockaj on 15. 10. 2025.
//

#include "SnmpOtelExport.h"

#include "../libs/httplib/httplib.h"

int SnmpOtelExport::SendHttpMessage(const std::string &target_ip, const std::string &message) {

    std::string base, path;

    parse_otlp_url(target_ip, base, path);

    httplib::Client cli(base);
    auto res = cli.Post(path, message, "application/json");

    if(res != nullptr)
        return res->status;
    else{
        return -1;
    }
}

int SnmpOtelExport::parse_otlp_url(const std::string &full_url, std::string &base, std::string &path)
{
    // Hlada /v1/metrics - alebo prvu cestu za hostom/portom
    size_t protocol_end = full_url.find("//");
    if (protocol_end == std::string::npos) {
        protocol_end = 0; // Zaciatok
    } else {
        protocol_end += 2;
    }

    size_t path_start = full_url.find('/', protocol_end);

    if (path_start != std::string::npos) {
        base = full_url.substr(0, path_start);
        path = full_url.substr(path_start);
    } else {
        base = full_url;
        path = "/";
    }
    return 0;
}
