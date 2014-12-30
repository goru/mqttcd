/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=0 : */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "MQTTPacket.h"
#include "transport.h"

#include "mqttcd_type.h"

int logger_open(mqttcd_context_t* context);
int logger_close(mqttcd_context_t* context);

void logger_error(mqttcd_context_t* context, const char* format, ...);
void logger_info(mqttcd_context_t* context, const char* format, ...);
void logger_debug(mqttcd_context_t* context, const char* format, ...);

static int MQTTCD_INTERRUPTED_SIGNAL = 0;
void signal_handler(int signum);

int parse_arguments(mqttcd_context_t* context, int argc, char** argv);
int free_arguments(mqttcd_context_t* context);

int mqtt_connect(mqttcd_context_t* context);
int mqtt_initialize_connection(mqttcd_context_t* context);
int mqtt_read_publish(mqttcd_context_t* context, char** payload);
int mqtt_send_ping(mqttcd_context_t* context);
int mqtt_finalize_connection(mqttcd_context_t* context);
void mqtt_disconnect(mqttcd_context_t* context);
