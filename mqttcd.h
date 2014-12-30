/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=0 : */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>

#include "MQTTPacket.h"
#include "transport.h"

#define BUFFER_LENGTH 1000

enum mqttcd_status {
    MQTTCD_SUCCEEDED,
    MQTTCD_SETUP_SIGNAL_FAILED,
    MQTTCD_PARSE_ARG_FAILED,
    MQTTCD_OPEN_FAILED,
    MQTTCD_SERIALIZE_FAILED,
    MQTTCD_DESERIALIZE_FAILED,
    MQTTCD_SEND_PACKET_FAILED,
    MQTTCD_PACKET_TYPE_MISMATCHED
};

typedef struct _mqttcd_raw_option {
    char* host;
    char* port;
    char* version;
    char* client_id;
    char* username;
    char* password;
    char* topic;
} mqttcd_raw_option_t;

typedef struct _mqttcd_option {
    mqttcd_raw_option_t raw_option;
    char* host;
    int port;
    int version;
    char* client_id;
    char* username;
    char* password;
    char* topic;
} mqttcd_option_t;

static int MQTTCD_INTERRUPTED_SIGNAL = 0;
void signal_handler(int signum);

int parse_arguments(int argc, char** argv, mqttcd_option_t* option);
int free_arguments(mqttcd_option_t* option);

int mqtt_connect(mqttcd_option_t* option, int* sock);
int mqtt_initialize_connection(int sock, mqttcd_option_t* option);
int mqtt_read_publish();
int mqtt_send_ping(int sock);
int mqtt_finalize_connection(int sock);
int mqtt_disconnect(int sock);
