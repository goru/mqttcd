/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=4 : */

#include "mqttcd_wrapper.h"

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
        logger_debug(context, "couldn't read publish packet\n");
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

    logger_notice(context, "%s\n", *payload);

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

