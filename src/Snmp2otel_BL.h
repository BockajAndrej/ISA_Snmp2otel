//
// Created by andrej.bockaj on 12. 10. 2025.
//

#ifndef SNMP2OTEL_SNMP2OTEL_BL_H
#define SNMP2OTEL_SNMP2OTEL_BL_H

namespace snmp2otel_bl {

    class Snmp2otel_BL {
    private: int PORT;
    private: int MAXLINE;
    private: int FLAGS;
    private: char* Packet;
    private: char* Message;

    public: Snmp2otel_BL();
    public: int mainFnc();

        int TmpFuc();
    };

} // snmp2otel_bl

#endif //SNMP2OTEL_SNMP2OTEL_BL_H
