/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=0 : */

#include "mqttcd.h"

int main(int argc, char** argv) {
    int ret;

    // setup signal handler
    if (SIG_ERR == signal(SIGINT, signal_handler)) {
        return MQTTCD_SETUP_SIGNAL_FAILED;
    }
    if (SIG_ERR == signal(SIGTERM, signal_handler)) {
        return MQTTCD_SETUP_SIGNAL_FAILED;
    }

    // parse command line arguments
    mqttcd_option_t option;
    ret = parse_arguments(argc, argv, &option);
    if (ret != MQTTCD_SUCCEEDED) {
        free_arguments(&option);
        return ret;
    }

    // connect to mqtt broker
    int sock;
    ret = mqtt_connect(&option, &sock);
    if (ret != MQTTCD_SUCCEEDED) {
        free_arguments(&option);
        return ret;
    }

    // initialize connection and subscribe mqtt topic
    ret = mqtt_initialize_connection(sock, &option);
    if (ret == MQTTCD_SUCCEEDED) {
        // receive loop
        while (MQTTCD_INTERRUPTED_SIGNAL == 0) {
            ret = mqtt_read_publish();
            ret = mqtt_send_ping(sock);
        }

        // send disconnect packet
        ret = mqtt_finalize_connection(sock);
    }

    // disconnect from mqtt broker
    ret = mqtt_disconnect(sock);
    if (ret != MQTTCD_SUCCEEDED) {
        free_arguments(&option);
        return ret;
    }

    free_arguments(&option);
    return MQTTCD_SUCCEEDED;
}

void signal_handler(int signum) {
    MQTTCD_INTERRUPTED_SIGNAL = 1;
}

int parse_arguments(int argc, char** argv, mqttcd_option_t* option) {
    struct option options[] = {
        { "host",      required_argument, NULL, 0 },
        { "port",      required_argument, NULL, 0 },
        { "version",   required_argument, NULL, 0 },
        { "client_id", required_argument, NULL, 0 },
        { "username",  required_argument, NULL, 0 },
        { "password",  required_argument, NULL, 0 },
        { "topic",     required_argument, NULL, 0 },
        { 0,           0,                 0,    0 }
    };

    char** raw_options[] = {
        &option->raw_option.host,
        &option->raw_option.port,
        &option->raw_option.version,
        &option->raw_option.client_id,
        &option->raw_option.username,
        &option->raw_option.password,
        &option->raw_option.topic
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
        }
    }

    // check parsed arguments
    if (option->raw_option.host != NULL) {
        option->host = option->raw_option.host;
    } else {
        return MQTTCD_PARSE_ARG_FAILED;
    }

    if (option->raw_option.port != NULL) {
        option->port = atoi(option->raw_option.port);
    } else {
        option->port = 1883; // default is 1883.
    }

    if (option->raw_option.version != NULL) {
        option->version = atoi(option->raw_option.version);
    } else {
        option->version = 3; // default is 3. 3 or 4.
    }

    if (option->raw_option.client_id != NULL) {
        option->client_id = option->raw_option.client_id;
    } else {
        return MQTTCD_PARSE_ARG_FAILED;
    }

    if (option->raw_option.username != NULL) {
        option->username = option->raw_option.username;
    } else {
        return MQTTCD_PARSE_ARG_FAILED;
    }

    if (option->raw_option.password != NULL) {
        option->password = option->raw_option.password;
    } else {
        return MQTTCD_PARSE_ARG_FAILED;
    }

    if (option->raw_option.topic != NULL) {
        option->topic = option->raw_option.topic;
    } else {
        return MQTTCD_PARSE_ARG_FAILED;
    }

    return MQTTCD_SUCCEEDED;
}

int free_arguments(mqttcd_option_t* option) {
    char** raw_options[] = {
        &option->raw_option.host,
        &option->raw_option.port,
        &option->raw_option.version,
        &option->raw_option.client_id,
        &option->raw_option.username,
        &option->raw_option.password,
        &option->raw_option.topic
    };

    for (int i = 0; i < sizeof(raw_options) / sizeof(raw_options[0]); i++) {
        if (*(raw_options[i]) != NULL) {
            free(*(raw_options[i]));
        }
    }

    return MQTTCD_SUCCEEDED;
}

int mqtt_connect(mqttcd_option_t* option, int* sock) {
    // connect to mqtt broker
    int s = transport_open(option->host, option->port);
    if(s < 0) {
        return MQTTCD_OPEN_FAILED;
    }

    *sock = s;

    return MQTTCD_SUCCEEDED;
}

int mqtt_initialize_connection(int sock, mqttcd_option_t* option) {
    unsigned char buf[BUFFER_LENGTH];
    int packet_len;
    int send_result;
    int packet_type;

    // create packet for connection data
    MQTTPacket_connectData conn_data = MQTTPacket_connectData_initializer;
    conn_data.MQTTVersion = option->version;
    conn_data.clientID.cstring = option->client_id;
    conn_data.keepAliveInterval = 20;
    conn_data.cleansession = 1;
    conn_data.username.cstring = option->username;
    conn_data.password.cstring = option->password;

    packet_len = MQTTSerialize_connect(buf, BUFFER_LENGTH, &conn_data);
    if (packet_len <= 0) {
        return MQTTCD_SERIALIZE_FAILED;
    }

    // send connection data
    send_result = transport_sendPacketBuffer(sock, buf, packet_len);
    if (send_result != packet_len) {
        return MQTTCD_SEND_PACKET_FAILED;
    }

    // read connack packet from broker
    packet_type = MQTTPacket_read(buf, BUFFER_LENGTH, transport_getdata);
    if (packet_type != CONNACK) {
        return MQTTCD_PACKET_TYPE_MISMATCHED;
    }

    // create subscribe packet
    int dup = 0;
    int msgid = 1;
    int count = 1;

    MQTTString topic= MQTTString_initializer;
    topic.cstring = option->topic;

    int qos = 0;

    packet_len = MQTTSerialize_subscribe(buf, BUFFER_LENGTH, dup, msgid, count, &topic, &qos);
    if (packet_len <= 0) {
        return MQTTCD_SERIALIZE_FAILED;
    }

    // send subscribe packet
    send_result = transport_sendPacketBuffer(sock, buf, packet_len);
    if (send_result != packet_len) {
        return MQTTCD_SEND_PACKET_FAILED;
    }

    // read suback packet from broker
    packet_type = MQTTPacket_read(buf, BUFFER_LENGTH, transport_getdata);
    if (packet_type != SUBACK) {
        return MQTTCD_PACKET_TYPE_MISMATCHED;
    }

    return MQTTCD_SUCCEEDED;
}

int mqtt_read_publish() {
    unsigned char buf[BUFFER_LENGTH];
    int packet_len;
    int send_result;
    int packet_type;

    // waiting for receiving publish packet
    // transport_getdata() has built-in 1 second timeout
    packet_type = MQTTPacket_read(buf, BUFFER_LENGTH, transport_getdata);
    if (packet_type != PUBLISH) {
        return MQTTCD_PACKET_TYPE_MISMATCHED;
    }

    // deserialize received publish packet
    unsigned char pub_dup;
    int qos;
    unsigned char retained;
    unsigned short pub_msgid;
    MQTTString receivedTopic;
    unsigned char* payload_in;
    int payloadlen_in;

    int result = MQTTDeserialize_publish(&pub_dup, &qos, &retained, &pub_msgid, &receivedTopic, &payload_in, &payloadlen_in, buf, BUFFER_LENGTH);
    if (result != 1) {
        return MQTTCD_DESERIALIZE_FAILED;
    }

    return MQTTCD_SUCCEEDED;
}

int mqtt_send_ping(int sock) {
    unsigned char buf[BUFFER_LENGTH];
    int packet_len;
    int send_result;

    // create ping packet
    packet_len = MQTTSerialize_pingreq(buf, BUFFER_LENGTH);
    if (packet_len <= 0) {
        return MQTTCD_SERIALIZE_FAILED;
    }

    // send ping packet for keep connection
    send_result = transport_sendPacketBuffer(sock, buf, packet_len);
    if (send_result != packet_len) {
        return MQTTCD_SEND_PACKET_FAILED;
    }

    return MQTTCD_SUCCEEDED;
}

int mqtt_finalize_connection(int sock) {
    unsigned char buf[BUFFER_LENGTH];
    int packet_len;
    int send_result;

    // create desconnect packet
    packet_len = MQTTSerialize_disconnect(buf, BUFFER_LENGTH);
    if (packet_len <= 0) {
        return MQTTCD_SERIALIZE_FAILED;
    }

    // send disconnect packet
    send_result = transport_sendPacketBuffer(sock, buf, packet_len);
    if (send_result != packet_len) {
        return MQTTCD_SEND_PACKET_FAILED;
    }

    return MQTTCD_SUCCEEDED;
}

int mqtt_disconnect(int sock) {
    // disconnect
    transport_close(sock);
    return MQTTCD_SUCCEEDED;
}

