//
// Created by andrej.bockaj on 15. 10. 2025.
//

#include "SnmpOtelExport.h"

#include "../libs/httplib/httplib.h"
#include "../libs/nlohmann/json.hpp"

std::string SnmpOtelExport::create_json(std::string msg) {
    nlohmann::json j;
    j["data"] = "testovacia_sprava";
    j["realneData"] = msg;
    return j.dump();
}

int SnmpOtelExport::SendHttpMessage(const std::string &target_ip, const std::string &message) {
    httplib::Client cli(target_ip);

    // Odoslanie POST požiadavky s Content-Type: application/json
    auto res = cli.Post("/", message, "application/json");

    if (res && res->status == 200) {
        std::cout << "Správa úspešne odoslaná. Odpoveď servera:\n" << res->body << std::endl;
    } else {
        std::cerr << "Chyba pri odosielaní: " << (res ? std::to_string(res->status) : "Žiadna odpoveď") << std::endl;
    }

    return res->status;
}
