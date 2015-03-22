/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=4 : */

#ifndef mqttcd_type_h
#define mqttcd_type_h

#include <stdio.h>

#define MQTTCD_BUFFER_LENGTH 4096
#define MQTTCD_KEEP_ALIVE    30
#define MQTTCD_PING_INTERVAL 25

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
    MQTTCD_PACKET_TYPE_MISMATCHED,
    MQTTCD_FCNTL_FAILED,
    MQTTCD_EXECUVE_FAILED
};

enum mqttcd_handler_type {
    MQTTCD_HANDLER_NOP,
    MQTTCD_HANDLER_STRING
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
    char* handler;
    char* handler_dir;
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
    int handler;
    char* handler_dir;
} mqttcd_option_t;

typedef struct _mqttcd_context {
    mqttcd_raw_option_t raw_option;
    mqttcd_option_t option;
    int mqtt_socket;
    FILE* logger;
} mqttcd_context_t;

#endif

