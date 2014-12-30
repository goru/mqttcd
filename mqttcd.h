/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=0 : */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "MQTTPacket.h"
#include "transport.h"

#include "mqttcd_type.h"
#include "mqttcd_logger.h"
#include "mqttcd_arg.h"

static int MQTTCD_INTERRUPTED_SIGNAL = 0;
void signal_handler(int signum);

int mqtt_connect(mqttcd_context_t* context);
int mqtt_initialize_connection(mqttcd_context_t* context);
int mqtt_read_publish(mqttcd_context_t* context, char** payload);
int mqtt_send_ping(mqttcd_context_t* context);
int mqtt_finalize_connection(mqttcd_context_t* context);
void mqtt_disconnect(mqttcd_context_t* context);
