/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=0 : */

#include "mqttcd.h"

int main(int argc, char** argv) {
    int ret;
    mqttcd_context_t context;

    // parse command line arguments
    ret = parse_arguments(&context, argc, argv);
    if (ret != MQTTCD_SUCCEEDED) {
        free_arguments(&context);
        return ret;
    }

    // daemonize
    if (context.option.daemonize == 1) {
        ret = fork();
        // fork is failed
        if (ret == -1) {
            free_arguments(&context);
            return MQTTCD_FORK_FAILED;
        }

        // exit parent process with child process number
        if (ret != 0) {
            printf("%d\n", ret);
            free_arguments(&context);
            return MQTTCD_SUCCEEDED;
        }

        // child process
        // close stdin, stdout, stderr
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        // create new session (disconnect from current terminal)
        setsid();
        // change working directory
        chdir("/");
        // reset umask
        umask(0);
        // ignore SIGCHLD for avoiding zombie process
        signal(SIGCHLD, SIG_IGN);
    }

    // setup signal handler
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        free_arguments(&context);
        return MQTTCD_SETUP_SIGNAL_FAILED;
    }
    if (signal(SIGTERM, signal_handler) == SIG_ERR) {
        free_arguments(&context);
        return MQTTCD_SETUP_SIGNAL_FAILED;
    }

    ret = logger_open(&context);
    if (ret != MQTTCD_SUCCEEDED) {
        free_arguments(&context);
        return ret;
    }

    // connect to mqtt broker
    ret = mqtt_connect(&context);
    if (ret != MQTTCD_SUCCEEDED) {
        logger_close(&context);
        free_arguments(&context);
        return ret;
    }

    // initialize connection and subscribe mqtt topic
    ret = mqtt_initialize_connection(&context);
    if (ret == MQTTCD_SUCCEEDED) {
        // receive loop
        int count = 0;
        while (MQTTCD_INTERRUPTED_SIGNAL == 0) {
            char* payload = NULL;
            ret = mqtt_read_publish(&context, &payload);
            if (ret == MQTTCD_SUCCEEDED && payload != NULL) {
                free(payload);
            }

            if (count++ > 30) {
                ret = mqtt_send_ping(&context);
                if (ret != MQTTCD_SUCCEEDED) {
                    break;
                }
                count = 0;
            }
        }

        // send disconnect packet
        ret = mqtt_finalize_connection(&context);
    }

    // disconnect from mqtt broker
    mqtt_disconnect(&context);

    // cleanup
    logger_close(&context);
    free_arguments(&context);

    return MQTTCD_SUCCEEDED;
}

int logger_open(mqttcd_context_t* context) {
    if (context->option.daemonize == 0) {
        context->logger = stdout;
    } else {
        FILE* fp = fopen("/var/tmp/mqttcd.log", "a");
        if (fp == NULL) {
            return MQTTCD_OPEN_FAILED;
        }
        context->logger = fp;
    }

    return MQTTCD_SUCCEEDED;
}

int logger_close(mqttcd_context_t* context) {
    if (context->option.daemonize == 0) {
        // do nothing
    } else {
        fclose(context->logger);
    }

    return MQTTCD_SUCCEEDED;
}

void logger_error(mqttcd_context_t* context, const char* format, ...) {
    va_list arg;
    va_start(arg, format);
    vfprintf(context->logger, format, arg);
    va_end(arg);
    fflush(context->logger);
}

void logger_info(mqttcd_context_t* context, const char* format, ...) {
    va_list arg;
    va_start(arg, format);
    vfprintf(context->logger, format, arg);
    va_end(arg);
    fflush(context->logger);
}

void logger_debug(mqttcd_context_t* context, const char* format, ...) {
    va_list arg;
    va_start(arg, format);
    vfprintf(context->logger, format, arg);
    va_end(arg);
    fflush(context->logger);
}

void signal_handler(int signum) {
    MQTTCD_INTERRUPTED_SIGNAL = 1;
}

int parse_arguments(mqttcd_context_t* context, int argc, char** argv) {
    struct option options[] = {
        { "host",      required_argument, NULL, 0 },
        { "port",      required_argument, NULL, 0 },
        { "version",   required_argument, NULL, 0 },
        { "client_id", required_argument, NULL, 0 },
        { "username",  required_argument, NULL, 0 },
        { "password",  required_argument, NULL, 0 },
        { "topic",     required_argument, NULL, 0 },
        { "daemonize", no_argument,       NULL, 0 },
        { 0,           0,                 0,    0 }
    };

    char** raw_options[] = {
        &context->raw_option.host,
        &context->raw_option.port,
        &context->raw_option.version,
        &context->raw_option.client_id,
        &context->raw_option.username,
        &context->raw_option.password,
        &context->raw_option.topic,
        &context->raw_option.daemonize
    };

    // initialize variables
    for (int i = 0; i < sizeof(raw_options) / sizeof(raw_options[0]); i++) {
        *(raw_options[i]) = NULL;
    }

    // parse command line arguments
    int result;
    int index;
    while ((result = getopt_long(argc, argv, "", options, &index)) != -1) {
        if (result != 0) {
            return MQTTCD_PARSE_ARG_FAILED;
        }

        if (options[index].has_arg != no_argument) {
            int length = strlen(optarg) + 1;
            *(raw_options[index]) = malloc(length);
            strncpy(*(raw_options[index]), optarg, length);
        } else {
            *(raw_options[index]) = malloc(1);
            (*(raw_options[index]))[0] = '\0';
        }
    }

    // check parsed arguments
    if (context->raw_option.host != NULL) {
        context->option.host = context->raw_option.host;
    } else {
        return MQTTCD_PARSE_ARG_FAILED;
    }

    if (context->raw_option.port != NULL) {
        context->option.port = atoi(context->raw_option.port);
    } else {
        context->option.port = 1883; // default is 1883.
    }

    if (context->raw_option.version != NULL) {
        context->option.version = atoi(context->raw_option.version);
    } else {
        context->option.version = 3; // default is 3. 3 or 4.
    }

    if (context->raw_option.client_id != NULL) {
        context->option.client_id = context->raw_option.client_id;
    } else {
        return MQTTCD_PARSE_ARG_FAILED;
    }

    if (context->raw_option.username != NULL) {
        context->option.username = context->raw_option.username;
    } else {
        return MQTTCD_PARSE_ARG_FAILED;
    }

    if (context->raw_option.password != NULL) {
        context->option.password = context->raw_option.password;
    } else {
        return MQTTCD_PARSE_ARG_FAILED;
    }

    if (context->raw_option.topic != NULL) {
        context->option.topic = context->raw_option.topic;
    } else {
        return MQTTCD_PARSE_ARG_FAILED;
    }

    if (context->raw_option.daemonize != NULL) {
        context->option.daemonize = 1;
    } else {
        context->option.daemonize = 0; // default is not daemonize
    }

    return MQTTCD_SUCCEEDED;
}

int free_arguments(mqttcd_context_t* context) {
    char** raw_options[] = {
        &context->raw_option.host,
        &context->raw_option.port,
        &context->raw_option.version,
        &context->raw_option.client_id,
        &context->raw_option.username,
        &context->raw_option.password,
        &context->raw_option.topic,
        &context->raw_option.daemonize
    };

    for (int i = 0; i < sizeof(raw_options) / sizeof(raw_options[0]); i++) {
        if (*(raw_options[i]) != NULL) {
            free(*(raw_options[i]));
        }
    }

    return MQTTCD_SUCCEEDED;
}

int mqtt_connect(mqttcd_context_t* context) {
    logger_debug(context, "connecting to mqtt broker... ");

    // connect to mqtt broker
    int sock = transport_open(context->option.host, context->option.port);
    if(sock < 0) {
        logger_error(context, "couldn't connect to mqtt broker: %d\n", sock);
        return MQTTCD_OPEN_FAILED;
    }

    context->mqtt_socket = sock;

    logger_debug(context, "ok\n");

    return MQTTCD_SUCCEEDED;
}

int mqtt_initialize_connection(mqttcd_context_t* context) {
    unsigned char buf[BUFFER_LENGTH];
    int packet_len;
    int send_result;
    int packet_type;

    // create packet for connection data
    MQTTPacket_connectData conn_data = MQTTPacket_connectData_initializer;
    conn_data.MQTTVersion = context->option.version;
    conn_data.clientID.cstring = context->option.client_id;
    conn_data.keepAliveInterval = 20;
    conn_data.cleansession = 1;
    conn_data.username.cstring = context->option.username;
    conn_data.password.cstring = context->option.password;

    logger_debug(context, "serializing connect packet... ");
    packet_len = MQTTSerialize_connect(buf, BUFFER_LENGTH, &conn_data);
    if (packet_len <= 0) {
        logger_error(context, "couldn't serialize connect packet: %d\n", packet_len);
        return MQTTCD_SERIALIZE_FAILED;
    }
    logger_debug(context, "ok\n");

    // send connection data
    logger_debug(context, "sending connect packet... ");
    send_result = transport_sendPacketBuffer(context->mqtt_socket, buf, packet_len);
    if (send_result != packet_len) {
        logger_error(context, "couldn't send connect packet: %d\n", send_result);
        return MQTTCD_SEND_PACKET_FAILED;
    }
    logger_debug(context, "ok\n");

    // read connack packet from broker
    logger_debug(context, "reading connack packet... ");
    packet_type = MQTTPacket_read(buf, BUFFER_LENGTH, transport_getdata);
    if (packet_type != CONNACK) {
        logger_error(context, "couldn't read connack packet: %d\n", packet_type);
        return MQTTCD_PACKET_TYPE_MISMATCHED;
    }
    logger_debug(context, "ok\n");

    // create subscribe packet
    int dup = 0;
    int msgid = 1;
    int count = 1;

    MQTTString topic= MQTTString_initializer;
    topic.cstring = context->option.topic;

    int qos = 0;

    logger_debug(context, "serializing subscribe packet... ");
    packet_len = MQTTSerialize_subscribe(buf, BUFFER_LENGTH, dup, msgid, count, &topic, &qos);
    if (packet_len <= 0) {
        logger_error(context, "couldn't serialize subscribe packet: %d\n", packet_len);
        return MQTTCD_SERIALIZE_FAILED;
    }
    logger_debug(context, "ok\n");

    // send subscribe packet
    logger_debug(context, "sending subscribe packet... ");
    send_result = transport_sendPacketBuffer(context->mqtt_socket, buf, packet_len);
    if (send_result != packet_len) {
        logger_error(context, "couldn't send subscribe packet: %d\n", send_result);
        return MQTTCD_SEND_PACKET_FAILED;
    }
    logger_debug(context, "ok\n");

    // read suback packet from broker
    logger_debug(context, "reading suback packet... ");
    packet_type = MQTTPacket_read(buf, BUFFER_LENGTH, transport_getdata);
    if (packet_type != SUBACK) {
        logger_error(context, "couldn't read suback packet: %d\n", packet_type);
        return MQTTCD_PACKET_TYPE_MISMATCHED;
    }
    logger_debug(context, "ok\n");

    return MQTTCD_SUCCEEDED;
}

int mqtt_read_publish(mqttcd_context_t* context, char** payload) {
    unsigned char buf[BUFFER_LENGTH];
    int packet_len;
    int send_result;
    int packet_type;

    // waiting for receiving publish packet
    // transport_getdata() has built-in 1 second timeout
    logger_debug(context, "reading publish packet... ");
    packet_type = MQTTPacket_read(buf, BUFFER_LENGTH, transport_getdata);
    if (packet_type == -1) {
        logger_error(context, "couldn't read publish packet\n");
        return MQTTCD_READ_PACKET_TIMEOUT;
    }
    if (packet_type != PUBLISH) {
        logger_error(context, "couldn't read publish packet: %d\n", packet_type);
        return MQTTCD_PACKET_TYPE_MISMATCHED;
    }
    logger_debug(context, "ok\n");

    // deserialize received publish packet
    unsigned char pub_dup;
    int qos;
    unsigned char retained;
    unsigned short pub_msgid;
    MQTTString receivedTopic;
    unsigned char* payload_in;
    int payloadlen_in;

    logger_debug(context, "deserializing publish packet... ");
    int result = MQTTDeserialize_publish(&pub_dup, &qos, &retained, &pub_msgid, &receivedTopic, &payload_in, &payloadlen_in, buf, BUFFER_LENGTH);
    if (result != 1) {
        logger_error(context, "couldn't deserialize publish packet: %d\n", result);
        return MQTTCD_DESERIALIZE_FAILED;
    }
    logger_debug(context, "ok\n");

    *payload = malloc(payloadlen_in + 1);
    strncpy(*payload, (const char*)payload_in, payloadlen_in);
    (*payload)[payloadlen_in] = '\0';

    logger_info(context, "received payload is: %s\n", *payload);

    return MQTTCD_SUCCEEDED;
}

int mqtt_send_ping(mqttcd_context_t* context) {
    unsigned char buf[BUFFER_LENGTH];
    int packet_len;
    int send_result;
    int packet_type;

    // create ping packet
    logger_debug(context, "serializing pingreq packet... ");
    packet_len = MQTTSerialize_pingreq(buf, BUFFER_LENGTH);
    if (packet_len <= 0) {
        logger_error(context, "couldn't serialize pingreq packet: %d\n", packet_len);
        return MQTTCD_SERIALIZE_FAILED;
    }
    logger_debug(context, "ok\n");

    // send ping packet for keep connection
    logger_debug(context, "sending pingreq packet... ");
    send_result = transport_sendPacketBuffer(context->mqtt_socket, buf, packet_len);
    if (send_result != packet_len) {
        logger_error(context, "couldn't send pingreq packet: %d\n", send_result);
        return MQTTCD_SEND_PACKET_FAILED;
    }
    logger_debug(context, "ok\n");

    // read pingresp packet from broker
    logger_debug(context, "reading pingresp packet... ");
    packet_type = MQTTPacket_read(buf, BUFFER_LENGTH, transport_getdata);
    if (packet_type != PINGRESP) {
        logger_error(context, "couldn't read pingresp packet: %d\n", packet_type);
        return MQTTCD_PACKET_TYPE_MISMATCHED;
    }
    logger_debug(context, "ok\n");

    return MQTTCD_SUCCEEDED;
}

int mqtt_finalize_connection(mqttcd_context_t* context) {
    unsigned char buf[BUFFER_LENGTH];
    int packet_len;
    int send_result;

    // create desconnect packet
    logger_debug(context, "serializing disconnect packet... ");
    packet_len = MQTTSerialize_disconnect(buf, BUFFER_LENGTH);
    if (packet_len <= 0) {
        logger_error(context, "couldn't serialize disconnect packet: %d\n", packet_len);
        return MQTTCD_SERIALIZE_FAILED;
    }
    logger_debug(context, "ok\n");

    // send disconnect packet
    logger_debug(context, "sending disconnect packet... ");
    send_result = transport_sendPacketBuffer(context->mqtt_socket, buf, packet_len);
    if (send_result != packet_len) {
        logger_error(context, "couldn't send disconnect packet: %d\n", send_result);
        return MQTTCD_SEND_PACKET_FAILED;
    }
    logger_debug(context, "ok\n");

    return MQTTCD_SUCCEEDED;
}

void mqtt_disconnect(mqttcd_context_t* context) {
    // disconnect
    logger_debug(context, "disconnect from mqtt broker... ");
    transport_close(context->mqtt_socket);
    logger_debug(context, "ok\n");
}

