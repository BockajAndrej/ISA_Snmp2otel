//
// Created by andrej.bockaj on 14. 10. 2025.
//
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

SNMP::StdByteVector SnmpOtelBer::CreateSnmpMsg(const std::string& CommunityString, int PduType, std::vector<std::string> *OidsRaw) {

    SNMP::OIDList normalizedOids;
    for(const std::string& strOid : *OidsRaw){
        normalizedOids.push_back(SNMP::OID(strOid));
    }

    SNMP::Encoder snmp;
    snmp.setupGetRequest( SnmpVersion, CommunityString, PduType, normalizedOids);
    return snmp.encodeRequest();
}

int SnmpOtelBer::DecodeData(SnmpOtelExportConfig &config, SNMP::StdByteVector bytes) {
    SNMP::Encoder snmp;
    if( !snmp.decodeAll(bytes, true) )    // second parameter 'true' to keep raw data. Usefull for debugging.
    {
        std::cerr << "Error " << snmp.errorCode() << " in object " << snmp.errorObjectIndex() << std::endl;
        return EXIT_FAILURE;
    }
    else
    {
        for( const auto &asn1Varbind : snmp.varbindList() )
        {
            switch( asn1Varbind.asn1Variable().type())
            {
                case ASN1TYPE_NULL:
                    std::cout << "<null>" << std::endl;
                    break;
                case ASN1TYPE_OCTETSTRING:
                case ASN1TYPE_IA5String:
                case ASN1TYPE_VideoString:
                    //		case ASN1TYPE_xxxxString:
                    // Beware. Never call octetString() to use data as a string because it doesn't includes the last /0 byte.
                    std::cout << "OctetString " <<  asn1Varbind.asn1Variable().toStdString() << std::endl;
                    break;
                case ASN1TYPE_INTEGER:
                case ASN1TYPE_Integer64:
                case ASN1TYPE_Unsigned64:
                case ASN1TYPE_Counter64:
                case ASN1TYPE_Gauge32:
                    //		case ASN1TYPE_numbers:
                    config.attributes.insert({"asInt", std::to_string(asn1Varbind.asn1Variable().toUnsigned64())});
                    break;
                default:
                    std::cout << "Unkown type: " << SNMP::Utils::printableByte(asn1Varbind.asn1Variable().type())
                              << "Raw data: " << SNMP::Utils::printableBytes(asn1Varbind.rawValue()) << std::endl;
            }
        }
    }
    return EXIT_SUCCESS;
}

