//
// Created by andrej.bockaj on 14. 10. 2025.
//

#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include <csignal>
#include <iostream>
#include <cstring>

#ifndef SNMP2OTEL_SIGNALHANDLER_H
#define SNMP2OTEL_SIGNALHANDLER_H


class SignalHandler {
public:
    static volatile std::sig_atomic_t signal_received;

    void register_handler();

private:
    static void static_signal_handler(int signum);
};

#endif //SNMP2OTEL_SIGNALHANDLER_H