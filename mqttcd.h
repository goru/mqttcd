/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=0 : */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "MQTTPacket.h"
#include "transport.h"

#define BUFFER_LENGTH 1000

enum mqttcd_status {
    MQTTCD_SUCCEEDED,
    MQTTCD_OPEN_FAILED,
    MQTTCD_SERIALIZE_FAILED,
    MQTTCD_DESERIALIZE_FAILED,
    MQTTCD_SEND_PACKET_FAILED,
    MQTTCD_PACKET_TYPE_MISMATCHED,
    MQTTCD_INTERRUPTED
};

typedef struct _mqttcd_option {
    char* host;
    int port;
    int version;
    char* client_id;
    char* username;
    char* password;
    char* topic;
} mqttcd_option_t;

int parse_arguments(int argc, char** argv, mqttcd_option_t* option);

int mqtt_connect(mqttcd_option_t* option, int* sock);
int mqtt_initialize_connection(int sock, mqttcd_option_t* option);
int mqtt_read_publish();
int mqtt_send_ping(int sock);
int mqtt_finalize_connection(int sock);
int mqtt_disconnect(int sock);
