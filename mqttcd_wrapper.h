/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=4 : */

#ifndef mqttcd_wrapper_h
#define mqttcd_wrapper_h

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "MQTTPacket.h"
#include "transport.h"

#include "mqttcd_type.h"
#include "mqttcd_logger.h"

int mqtt_connect(mqttcd_context_t* context);
void mqtt_disconnect(mqttcd_context_t* context);

int mqtt_send(mqttcd_context_t* context, unsigned char* buf, int length);
int mqtt_recv(mqttcd_context_t* context, unsigned char* buf, int length, int* packet_type);

int mqtt_initialize_connection(mqttcd_context_t* context);
int mqtt_finalize_connection(mqttcd_context_t* context);

int mqtt_read_publish(mqttcd_context_t* context, char** payload);
int mqtt_send_ping(mqttcd_context_t* context);

#endif

