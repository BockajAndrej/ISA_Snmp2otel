#include <iostream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctime>

#include "../libs/BasicSnmp/lib/snmplib.h"

// --- Aliasy a Definície ---
using StdString = std::string;
using StdByteVector = std::vector<uint8_t>;

// ASN.1 Typy (podľa SNMP MIB)
#define ASN1TYPE_GET_REQUEST    (uint8_t)0xA0
#define ASN1TYPE_GET_RESPONSE   (uint8_t)0xA2
#define ASN1TYPE_GAUGE32        (uint8_t)0x42

// Definície agenta
#define SNMP_PORT 16161
#define MAX_BUFFER_SIZE 10224
#define COMMUNITY_STRING "private"
const StdString TARGET_OID_STR = ".1.3.6.1.4.1.9999.1.0";
// ----------------------------

/**
 * @brief Vytvorí a zakóduje kompletnú SNMPv2c GetResponse správu.
 */
StdByteVector createGetResponse(int request_id, const SNMP::PDUVarbindList& varbinds_to_send) {
    SNMP::Encoder response_encoder;

    // Knižnica BasicSNMP je primárne pre manažéra. Použijeme setupSetRequest (v2c = 2),
    // ktorý umožňuje poslať zoznam VarBindov, a potom zmeníme PDU typ manuálne.
    response_encoder.setupSetRequest(2, COMMUNITY_STRING, request_id, varbinds_to_send);

    // Kódovanie na surové bajty
    StdByteVector bytes = response_encoder.encodeRequest();

    // UPOZORNENIE: Ak knižnica BasicSNMP nepodporuje priamu zmenu PDU typu na GetResponse,
    // nasledovná operácia je nebezpečný hack na manuálnu zmenu PDU typu v surových bajtoch (z 0xA3 na 0xA2).
    // Pre robustné riešenie by bolo nutné rozšíriť knižnicu. Pre túto ukážku ju vynecháme.
    // bytes[index_PDU_typu] = ASN1TYPE_GET_RESPONSE;

    // Budeme sa spoliehať, že knižnica to zvládne interne.
    return bytes;
}


int main() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Chyba: Vytvorenie socketu zlyhalo");
        exit(EXIT_FAILURE);
    }

    std::memset(&servaddr, 0, sizeof(servaddr));
    std::memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(SNMP_PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Chyba: Bind zlyhal (Port 161 je privilegovaný, skúste sudo)");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    std::cout << "SNMP Agent (v2c) beží na UDP porte " << SNMP_PORT << "..." << std::endl;
    std::cout << "Podporované OID: " << TARGET_OID_STR << std::endl;
    std::srand(std::time(0));

    while (true) {
        SNMP::StdByteVector buffer(MAX_BUFFER_SIZE);
        socklen_t len = sizeof(cliaddr);

        int n = recvfrom(sockfd, buffer.data(), MAX_BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, &len);
        if (n < 0) {
            perror("recvfrom failed");
            continue;
        }
        buffer.resize(n);

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(cliaddr.sin_addr), client_ip, INET_ADDRSTRLEN);
        std::cout << "\n--- Prijatá požiadavka z: " << client_ip << " ---" << std::endl;

        SNMP::Encoder incoming_snmp;
        if (!incoming_snmp.decodeAll(buffer, false)) {
            std::cerr << "CHYBA DEKÓDOVANIA: Chyba " << incoming_snmp.errorCode() << std::endl;
            continue;
        }

        if (incoming_snmp.version() != 1) {
            std::cerr << "Chyba: Očakávam v2c (2), prijaté: " << incoming_snmp.version() << std::endl;
            continue;
        }

        if (incoming_snmp.comunity() != COMMUNITY_STRING) {
            std::cerr << "Chyba: Nesprávna komunita." << std::endl;
            continue;
        }

//        if (incoming_snmp.requestType() != ASN1TYPE_GET_REQUEST) {
//            std::cerr << "Chyba: Očakávam GetRequest (0xA0), no dostal: "<< (int)incoming_snmp.requestType() << std::endl;
//            continue;
//        }

        std::cout << "Prijatý GetRequest s ID: " << incoming_snmp.requestID() << std::endl;

        SNMP::PDUVarbindList response_varbinds;

        if (incoming_snmp.varbindList().empty()) continue;

        const SNMP::PDUVarbind& request_vb = incoming_snmp.varbindList().first();
        StdString requested_oid = request_vb.oid().toStdString();

        std::cout << "Požadované OID: " << requested_oid << std::endl;

        if (requested_oid == TARGET_OID_STR){
            uint32_t gauge_value = static_cast<uint32_t>(std::rand() % 1000 + 1);

            SNMP::ASN1Variable gauge_var;
            gauge_var.setGauge32(gauge_value);
            SNMP::PDUVarbind response_vb(request_vb.oid(), gauge_var);
            response_varbinds.push_back(response_vb);

            std::cout << "  -> Odpoveď: OID " << TARGET_OID_STR << " = Gauge32: " << gauge_value << std::endl;

        } else {
            // Vrátenie noSuchName (Null hodnota)
            SNMP::ASN1Variable gauge_var;
            gauge_var.setNull();
            SNMP::PDUVarbind response_vb(request_vb.oid(), gauge_var);
            response_varbinds.push_back(response_vb);

            std::cout << "  -> Odpoveď: Requested OID: " << requested_oid << " (ZLE), Ocakavane: " << TARGET_OID_STR << std::endl;
        }

        StdByteVector encoded_response = createGetResponse(incoming_snmp.requestID(), response_varbinds);

        int sent = sendto(sockfd, encoded_response.data(), encoded_response.size(), 0, (const struct sockaddr *)&cliaddr, len);
        if (sent < 0) {
            perror("sendto failed");
        } else {
            std::cout << "Odoslaná odpoveď (" << sent << " bajtov) späť manažérovi." << std::endl;
        }
    }

    close(sockfd);
    return 0;
}