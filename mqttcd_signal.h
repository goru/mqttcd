/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=4 : */

#include <signal.h>

#include "mqttcd_type.h"

static int MQTTCD_SIGNAL_INTERRUPTED = 0;

int setup_signal_handler();
void signal_handler(int signum);
int signal_interrupted();

