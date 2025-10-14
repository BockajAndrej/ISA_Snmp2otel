//
// Created by andrej.bockaj on 13. 10. 2025.
//

#include <iostream>
#include <vector>
#include <sstream>

#include "snmpBer.h"

namespace snmpber {
    snmpBer::snmpBer(int SnmpVersion) {
        this->SnmpVersion = SnmpVersion;

        RequestID = 1;
        Error = 0;
        ErrorIndex = 0;
    }

    std::vector<unsigned char> snmpBer::CreateSnmpMessage(std::string CommunityString, int PduType, std::vector<std::string> *OidArray) {
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

        msg[SizeValue-1] = msg.size() - SizeValue;

        return msg;
    }

    void snmpBer::CreateSnmpPdu(std::vector<unsigned char> *msg, char PduType, std::vector<std::string> *OidArray) {
        msg->push_back(PduType);
        msg->push_back(0); //NULL
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

        (*msg)[SizeValue-1] = msg->size() - SizeValue;
    }

    void snmpBer::CreateVarbindList(std::vector<unsigned char> *msg, std::vector<std::string> *OidArray) {
        //Varbind List
        msg->push_back(TLV_T::SEQUENCE);
        msg->push_back(0);
        int SizeValue = msg->size();

        //Varbind
        for(std::string oid : *OidArray){
            CreateVarbind(msg, oid);
        }

        (*msg)[SizeValue-1] = msg->size() - SizeValue;
    }

    void snmpBer::CreateVarbind(std::vector<unsigned char> *msg, std::string OidStr) {
        //Varbind
        msg->push_back(TLV_T::SEQUENCE);
        msg->push_back(0);
        int SizeValue = msg->size();
        //OID
        std::vector<uint8_t> oids = OidToVector(OidStr);

        for(uint8_t i : oids){
            std::cout << std::dec << "2Num: " << (int)i << std::endl;
            msg->push_back(i);
        }

        //Value
        msg->push_back(TLV_T::NULL_TYPE);
        msg->push_back(0);

        (*msg)[SizeValue-1] = msg->size() - SizeValue;
    }

    std::vector<std::string> snmpBer::TrimString(const std::string &vstup, char delimiter) {
        std::vector<std::string> tokeny;
        std::string token;

        std::stringstream ss(vstup);

        while (std::getline(ss, token, delimiter)) {
            tokeny.push_back(token);
        }

        return tokeny;
    }

    std::vector<uint8_t> snmpBer::IntegerToBytesBigEndian(uint32_t number) {
        std::vector<uint8_t> byty;

        // Expected Big-Endian data
        uint8_t temp_byty[4];
        temp_byty[0] = (uint8_t)((number >> 24) & 0xFF); // MSB
        temp_byty[1] = (uint8_t)((number >> 16) & 0xFF);
        temp_byty[2] = (uint8_t)((number >> 8) & 0xFF);
        temp_byty[3] = (uint8_t)(number & 0xFF);         // LSB

        // Find first signified byte
        int first_significant_byte_index = -1;
        for (int i = 0; i < 4; ++i) {
            if (temp_byty[i] != 0x00) {
                first_significant_byte_index = i;
                break;
            }
        }

        if (first_significant_byte_index == -1) {
            byty.push_back(0x00);
            return byty;
        }

        // Divide positive and negative with 0x00
        if (temp_byty[first_significant_byte_index] & 0x80) {
            byty.push_back(0x00);
        }

        // Save from MSB
        for (int i = first_significant_byte_index; i < 4; ++i) {
            byty.push_back(temp_byty[i]);
        }

        return byty;
    }

    std::vector<uint32_t> snmpBer::DivideOid(const std::string& oid_string) {
        std::vector<uint32_t> segmenty;
        std::stringstream ss(oid_string);
        std::string segment;

        while (std::getline(ss, segment, '.')) {
            if (!segment.empty()) {
                try {
                    segmenty.push_back(std::stoul(segment));
                } catch (const std::exception& e) {
                    throw std::invalid_argument("Neplatny segment OID: " + segment);
                }
            }
        }
        if (segmenty.size() < 2) {
            throw std::invalid_argument("OID musi mat aspon dva segmenty.");
        }
        return segmenty;
    }
    void snmpBer::EncodeSegment(uint32_t value, std::vector<uint8_t>& result_bytes) {
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
    std::vector<uint8_t> snmpBer::OidToVector(const std::string& oid_string) {
        std::vector<uint32_t> segmenty = DivideOid(oid_string);
        std::vector<uint8_t> encoded_data;

        // First byte (X * 40) + Y
        uint32_t prvy_byt_hodnota = segmenty[0] * 40 + segmenty[1];

        //(X*40)+Y > 255
        if (prvy_byt_hodnota > 0xFF) {
            throw std::invalid_argument("Prvy segment (X*40+Y) je prilis velky.");
        }

        encoded_data.push_back(static_cast<uint8_t>(prvy_byt_hodnota));

        // Others semgments
        for (size_t i = 2; i < segmenty.size(); ++i) {
            EncodeSegment(segmenty[i], encoded_data);
        }

        std::vector<uint8_t> final_vector;

        if (encoded_data.size() > 127) {
            throw std::runtime_error("OID je prilis dlhy na jednobyte kódovanie dĺžky.");
        }

        final_vector.push_back(0x06);
        final_vector.push_back(static_cast<unsigned char>(encoded_data.size()));
        final_vector.insert(final_vector.end(), encoded_data.begin(), encoded_data.end());

        return final_vector;
    }
} // snmpber