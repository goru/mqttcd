/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=4 : */

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdlib.h>

#include "mqttcd_type.h"
#include "mqttcd_logger.h"
#include "mqttcd_arg.h"
#include "mqttcd_wrapper.h"

static int MQTTCD_INTERRUPTED_SIGNAL = 0;
void signal_handler(int signum);
