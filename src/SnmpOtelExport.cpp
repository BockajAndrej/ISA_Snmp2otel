//
// Created by andrej.bockaj on 15. 10. 2025.
//

#include "SnmpOtelExport.h"

#include "../libs/httplib/httplib.h"

int SnmpOtelExport::SendHttpMessage(const std::string &target_ip, const std::string &message) {

    httplib::Client cli(target_ip);
    auto res = cli.Post("/", message, "application/json");

    if(res != nullptr)
        return res->status;
    else{
        return -1;
    }
}
