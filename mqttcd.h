/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=0 : */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mqttcd_type.h"
#include "mqttcd_logger.h"
#include "mqttcd_arg.h"
#include "mqttcd_wrapper.h"

static int MQTTCD_INTERRUPTED_SIGNAL = 0;
void signal_handler(int signum);
