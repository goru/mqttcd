/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=4 : */

#include "mqttcd_signal.h"

int setup_signal_handler() {
    MQTTCD_SIGNAL_INTERRUPTED = 0;

    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        return MQTTCD_SETUP_SIGNAL_FAILED;
    }
    if (signal(SIGTERM, signal_handler) == SIG_ERR) {
        return MQTTCD_SETUP_SIGNAL_FAILED;
    }

    return MQTTCD_SUCCEEDED;
}

void signal_handler(int signum) {
    MQTTCD_SIGNAL_INTERRUPTED = 1;
}

int signal_interrupted() {
  return MQTTCD_SIGNAL_INTERRUPTED;
}

