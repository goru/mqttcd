/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=4 : */

#ifndef mqttcd_arg_h
#define mqttcd_arg_h

#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "mqttcd_type.h"

int parse_arguments(mqttcd_context_t* context, int argc, char** argv);
int free_arguments(mqttcd_context_t* context);

#endif

