/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=4 : */

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
        int pid;
        ret = fork_current_process(&pid);
        if (ret != MQTTCD_SUCCEEDED) {
            free_arguments(&context);
            return ret;
        }

        // exit parent process with child process number
        if (pid != 0) {
            printf("%d\n", pid);
            free_arguments(&context);
            return MQTTCD_SUCCEEDED;
        }
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
    if (ret != MQTTCD_SUCCEEDED) {
        logger_close(&context);
        free_arguments(&context);
        return ret;
    }

    // receive loop
    int count = 0;
    while (MQTTCD_INTERRUPTED_SIGNAL == 0) {
        unsigned char buf[MQTTCD_BUFFER_LENGTH];
        int packet_type;
        ret = mqtt_recv(&context, buf, MQTTCD_BUFFER_LENGTH, &packet_type);
        if (ret == MQTTCD_RECV_FAILED) {
            logger_close(&context);
            free_arguments(&context);
            return ret;
        }

        if (ret == MQTTCD_RECV_TIMEOUT) {
            if (count++ > MQTTCD_PING_INTERVAL) {
                ret = mqtt_send_ping(&context);
                if (ret != MQTTCD_SUCCEEDED) {
                    break;
                }
                count = 0;
            }
            continue;
        }

        // if (ret == MQTTCD_SUCCEEDED)
        count = 0;

        if (packet_type == PUBLISH) {
            char* payload = NULL;
            ret = mqtt_deserialize_publish(&context, buf, MQTTCD_BUFFER_LENGTH, &payload);
            if (ret == MQTTCD_SUCCEEDED && payload != NULL) {
                free(payload);
            }
        }
    }

    // send disconnect packet
    ret = mqtt_finalize_connection(&context);

    // disconnect from mqtt broker
    mqtt_disconnect(&context);

    // cleanup
    logger_close(&context);
    free_arguments(&context);

    return MQTTCD_SUCCEEDED;
}

void signal_handler(int signum) {
    MQTTCD_INTERRUPTED_SIGNAL = 1;
}

