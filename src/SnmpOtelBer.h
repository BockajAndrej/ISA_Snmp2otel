//
// Created by andrej.bockaj on 14. 10. 2025.
//

#ifndef SNMP2OTEL_SNMPOTELBER_H
#define SNMP2OTEL_SNMPOTELBER_H

#include <string>
#include <vector>

class SnmpOtelBer {
public:
    SnmpOtelBer(int SnmpVersion);
    std::vector<unsigned char> CreateSnmpMessage(const std::string& CommunityString, int PduType, std::vector<std::string> *OidArray);
    std::vector<unsigned char> procesuj_paket(const std::string& CommunityString, int PduType, std::vector<std::string> *OidsRaw);

private:
    int RequestID;
    int Error;
    int ErrorIndex;
    char SnmpVersion;

    enum TLV_T{
        BOOLEAN = 0b00000001,
        INTEGER = 0b00000010,
        OCTETSTRING = 0b00000100,
        NULL_TYPE = 0b00000101,
        OBJECTIDENTIFIER = 0b00000110,
        SEQUENCE = 0b00110000
    };

    std::vector<uint32_t> DivideOid(const std::string& oid_string);
    void EncodeSegment(uint32_t value, std::vector<uint8_t>& result_bytes);
    std::vector<uint8_t> OidToVector(const std::string& oid_string);

    void CreateSnmpPdu(std::vector<unsigned char> *msg, char PduType, std::vector<std::string> *OidArray);
    void CreateVarbindList(std::vector<unsigned char> *msg, std::vector<std::string> *OidArray);
    void CreateVarbind(std::vector<unsigned char>* msg, std::string OidStr);
};


#endif //SNMP2OTEL_SNMPOTELBER_H
