/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=4 : */

#ifndef mqttcd_type_h
#define mqttcd_type_h

#include <stdio.h>

#define BUFFER_LENGTH 4096

enum mqttcd_status {
    MQTTCD_SUCCEEDED,
    MQTTCD_FORK_FAILED,
    MQTTCD_SETUP_SIGNAL_FAILED,
    MQTTCD_PARSE_ARG_FAILED,
    MQTTCD_OPEN_FAILED,
    MQTTCD_SERIALIZE_FAILED,
    MQTTCD_DESERIALIZE_FAILED,
    MQTTCD_SEND_FAILED,
    MQTTCD_RECV_FAILED,
    MQTTCD_RECV_TIMEOUT,
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

#endif

