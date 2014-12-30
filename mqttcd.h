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

#define BUFFER_LENGTH 4096

enum mqttcd_status {
    MQTTCD_SUCCEEDED,
    MQTTCD_FORK_FAILED,
    MQTTCD_SETUP_SIGNAL_FAILED,
    MQTTCD_PARSE_ARG_FAILED,
    MQTTCD_OPEN_FAILED,
    MQTTCD_SERIALIZE_FAILED,
    MQTTCD_DESERIALIZE_FAILED,
    MQTTCD_SEND_PACKET_FAILED,
    MQTTCD_READ_PACKET_TIMEOUT,
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
    char* daemonize;
} mqttcd_raw_option_t;

typedef struct _mqttcd_option {
    char* host;
    int port;
    int version;
    char* client_id;
    char* username;
    char* password;
    char* topic;
    int daemonize;
} mqttcd_option_t;

typedef struct _mqttcd_context {
    mqttcd_raw_option_t raw_option;
    mqttcd_option_t option;
    int mqtt_socket;
    FILE* logger;
} mqttcd_context_t;

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
