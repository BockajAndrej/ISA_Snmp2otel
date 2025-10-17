//
// Created by andrej.bockaj on 17. 10. 2025.
//

#ifndef SNMP2OTEL_SNMPOTELCLIENT_H
#define SNMP2OTEL_SNMPOTELCLIENT_H

#include <string>
#include <vector>

class SnmpOtelClient {
public:
    int UpdateData(std::vector<unsigned char> &receivedMsg, const std::string& target_ip, const std::string& target_port, const std::vector<unsigned char>& message, struct timeval &tv);

private:
    int MAXLEN = 100;
};


#endif //SNMP2OTEL_SNMPOTELCLIENT_H
