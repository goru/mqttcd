/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=0 : */

#include "mqttcd.h"

int main(int argc, char** argv) {
    int ret;

    option_t option;
    ret = parse_arguments(argc, argv, &option);
    if (ret != MQTTCD_SUCCEEDED) {
        return ret;
    }

    int sock;
    ret = mqtt_connect(&option, &sock);
    if (ret != MQTTCD_SUCCEEDED) {
        return ret;
    }

    ret = mqtt_initialize_connection(sock, &option);
    if (ret == MQTTCD_SUCCEEDED) {
        while (1) {
            ret = mqtt_read_publish();
            ret = mqtt_send_ping(sock);
        }

        ret = mqtt_finalize_connection(sock);
    }

    ret = mqtt_disconnect(sock);
    if (ret != MQTTCD_SUCCEEDED) {
        return ret;
    }

    return MQTTCD_SUCCEEDED;
}

int parse_arguments(int argc, char** argv, option_t* option) {
    return MQTTCD_SUCCEEDED;
}

int mqtt_connect(option_t* option, int* sock) {
    // connect to mqtt broker
    int s = transport_open(option->host, option->port);
    if(s < 0) {
        return MQTTCD_OPEN_FAILED;
    }

    *sock = s;

    return MQTTCD_SUCCEEDED;
}

int mqtt_initialize_connection(int sock, option_t* option) {
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

    // send connection options
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

    // send ping packet for keep connection
    packet_len = MQTTSerialize_pingreq(buf, BUFFER_LENGTH);
    if (packet_len <= 0) {
        return MQTTCD_SERIALIZE_FAILED;
    }

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

