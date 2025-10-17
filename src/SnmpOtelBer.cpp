//
// Created by andrej.bockaj on 14. 10. 2025.
//
#include <sstream>

#include <string>
#include <iostream>
#include "../libs/BasicSnmp/lib/snmplib.h"

#include "SnmpOtelBer.h"

SnmpOtelBer::SnmpOtelBer(int SnmpVersion) {
    this->SnmpVersion = SnmpVersion;

    RequestID = 1;
    Error = 0;
    ErrorIndex = 0;
}
std::vector<unsigned char> SnmpOtelBer::CreateSnmpMessage(const std::string& CommunityString, int PduType, std::vector<std::string> *OidArray) {
    std::vector<unsigned char> msg;
    msg.push_back(TLV_T::SEQUENCE);
    msg.push_back(0);
    int SizeValue = msg.size();

    //Version
    msg.push_back(TLV_T::INTEGER);
    msg.push_back(1);
    msg.push_back(SnmpVersion);

    //Community string
    msg.push_back(TLV_T::OCTETSTRING);
    msg.push_back(CommunityString.size());
    for (char i : CommunityString) {
        msg.push_back(i);
    }

    //SNMP PDU
    CreateSnmpPdu(&msg, PduType, OidArray);

    //Fill sequence size
    msg[SizeValue-1] = msg.size() - SizeValue;

    return msg;
}

void SnmpOtelBer::CreateSnmpPdu(std::vector<unsigned char> *msg, char PduType, std::vector<std::string> *OidArray) {
    //PDU Header
    msg->push_back(PduType);
    msg->push_back(0);
    int SizeValue = msg->size();

    //RequestId
    msg->push_back(TLV_T::INTEGER);
    msg->push_back(1);
    msg->push_back(RequestID);

    //Error
    msg->push_back(TLV_T::INTEGER);
    msg->push_back(1);
    msg->push_back(Error);

    //Error index
    msg->push_back(TLV_T::INTEGER);
    msg->push_back(1);
    msg->push_back(ErrorIndex);

    //Varbind List
    CreateVarbindList(msg, OidArray);

    //Fill sequence size
    (*msg)[SizeValue-1] = msg->size() - SizeValue;
}
void SnmpOtelBer::CreateVarbindList(std::vector<unsigned char> *msg, std::vector<std::string> *OidArray) {
    //Varbind List Header
    msg->push_back(TLV_T::SEQUENCE);
    msg->push_back(0);
    int SizeValue = msg->size();

    //Varbind
    for(std::string oid : *OidArray){
        CreateVarbind(msg, oid);
    }
    //Fill sequence size
    (*msg)[SizeValue-1] = msg->size() - SizeValue;
}
void SnmpOtelBer::CreateVarbind(std::vector<unsigned char> *msg, std::string OidStr) {
    //Varbind Header
    msg->push_back(TLV_T::SEQUENCE);
    msg->push_back(0);
    int SizeValue = msg->size();
    //OID
    std::vector<uint8_t> oids = OidToVector(OidStr);

    for(uint8_t i : oids){
        msg->push_back(i);
    }

    //Value
    msg->push_back(TLV_T::NULL_TYPE);
    msg->push_back(0);

    //Fill sequence size
    (*msg)[SizeValue-1] = msg->size() - SizeValue;
}

std::vector<uint8_t> SnmpOtelBer::OidToVector(const std::string &oid_string) {
    std::vector<uint32_t> segmenty = DivideOid(oid_string);
    std::vector<uint8_t> encoded_data;

    // First byte (X * 40) + Y
    uint32_t prvy_byt_hodnota = segmenty[0] * 40 + segmenty[1];

    //(X*40)+Y > 255
    if (prvy_byt_hodnota > 0xFF) {
        throw std::invalid_argument("First segment (X*40+Y) is too big.");
    }

    encoded_data.push_back(static_cast<uint8_t>(prvy_byt_hodnota));

    // Others semgments
    for (size_t i = 2; i < segmenty.size(); ++i) {
        EncodeSegment(segmenty[i], encoded_data);
    }

    std::vector<uint8_t> final_vector;

    if (encoded_data.size() > 127) {
        throw std::runtime_error("OID is too long for 1B lengths.");
    }

    final_vector.push_back(0x06);
    final_vector.push_back(static_cast<unsigned char>(encoded_data.size()));
    final_vector.insert(final_vector.end(), encoded_data.begin(), encoded_data.end());

    return final_vector;
}
void SnmpOtelBer::EncodeSegment(uint32_t value, std::vector<uint8_t> &result_bytes) {
    if (value == 0) {
        result_bytes.push_back(0);
        return;
    }

    std::vector<uint8_t> temp_bytes;

    temp_bytes.push_back(static_cast<uint8_t>(value & 0x7F));
    value >>= 7;

    while (value > 0) {
        temp_bytes.push_back(static_cast<uint8_t>((value & 0x7F) | 0x80));
        value >>= 7;
    }

    std::reverse(temp_bytes.begin(), temp_bytes.end());

    result_bytes.insert(result_bytes.end(), temp_bytes.begin(), temp_bytes.end());
}
std::vector<uint32_t> SnmpOtelBer::DivideOid(const std::string &oid_string) {
    std::vector<uint32_t> segmenty;
    std::stringstream ss(oid_string);
    std::string segment;

    while (std::getline(ss, segment, '.')) {
        if (!segment.empty()) {
            try {
                segmenty.push_back(std::stoul(segment));
            } catch (const std::exception& e) {
                throw std::invalid_argument("Invalid segment OID: " + segment);
            }
        }
    }
    if (segmenty.size() < 2) {
        throw std::invalid_argument("OID need to have at least 2 segments.");
    }
    return segmenty;
}

std::vector<unsigned char> SnmpOtelBer::procesuj_paket(const std::string& CommunityString, int PduType, std::vector<std::string> *OidsRaw) {

    SNMP::OIDList Oids;
    for(const std::string& strOid : *OidsRaw){
        Oids.push_back(SNMP::OID(strOid));
    }

    SNMP::Encoder snmp;
    snmp.setupGetRequest( 1, CommunityString, PduType, Oids);
    SNMP::StdByteVector bytes = snmp.encodeRequest();
    return bytes;
}