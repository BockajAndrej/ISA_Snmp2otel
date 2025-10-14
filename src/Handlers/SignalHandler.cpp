//
// Created by andrej.bockaj on 14. 10. 2025.
//

#include "SignalHandler.h"

volatile std::sig_atomic_t SignalHandler::signal_received = 0;

void SignalHandler::static_signal_handler(int signum) {
    if (signum == SIGINT) {
        std::cout << "CTRL + C stlacene !!!!!!!!" << std::endl;
        signal_received = 1;
    }
}

void SignalHandler::register_handler() {
    if (std::signal(SIGINT, SignalHandler::static_signal_handler) == SIG_ERR) {
        std::cerr << "Chyba: Nepodarilo sa nastaviť obsluhu signálu." << std::endl;
    }
}
