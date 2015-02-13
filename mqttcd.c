/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=4 : */

#include "mqttcd.h"

int main(int argc, char** argv) {
    mqttcd_context_t context;

    // parse command line arguments
    int ret = parse_arguments(&context, argc, argv);
    if (ret != MQTTCD_SUCCEEDED) {
        goto cleanup;
    }

    // daemonize
    if (context.option.daemonize == 1) {
        int pid;
        ret = fork_current_process(&pid);
        if (ret != MQTTCD_SUCCEEDED) {
            goto cleanup;
        }

        // exit parent process with child process number
        if (pid != 0) {
            printf("%d\n", pid);
            ret = MQTTCD_SUCCEEDED;
            goto cleanup;
        }
    }

    // setup signal handler
    ret = setup_signal_handler();
    if (ret != MQTTCD_SUCCEEDED) {
        goto cleanup;
    }

    // start mqttcd
    ret = logger_open(&context);
    if (ret == MQTTCD_SUCCEEDED) {
        ret = mqttcd(&context);
        logger_close(&context);
    }

cleanup:
    // free parsed arguments
    free_arguments(&context);

    return ret;
}

int mqttcd(mqttcd_context_t* context) {
    // connect to mqtt broker
    int ret = mqtt_connect(context);
    if (ret != MQTTCD_SUCCEEDED) {
        goto disconnect;
    }

    // initialize connection and subscribe mqtt topic
    ret = mqtt_initialize_connection(context);
    if (ret != MQTTCD_SUCCEEDED) {
        goto disconnect;
    }

    // receive loop
    int count = 0;
    while (signal_interrupted() == 0) {
        unsigned char buf[MQTTCD_BUFFER_LENGTH];
        int packet_type;
        ret = mqtt_recv(context, buf, MQTTCD_BUFFER_LENGTH, &packet_type);
        if (ret == MQTTCD_RECV_FAILED) {
            goto disconnect;
        }

        if (ret == MQTTCD_RECV_TIMEOUT) {
            if (count++ > MQTTCD_PING_INTERVAL) {
                ret = mqtt_send_ping(context);
                if (ret != MQTTCD_SUCCEEDED) {
                    goto disconnect;
                }
                count = 0;
            }
            continue;
        }

        // if (ret == MQTTCD_SUCCEEDED)
        count = 0;

        if (packet_type == PUBLISH) {
            char* payload = NULL;
            ret = mqtt_deserialize_publish(context, buf, MQTTCD_BUFFER_LENGTH, &payload);
            if (ret == MQTTCD_SUCCEEDED && payload != NULL) {
                free(payload);
            }
        }
    }

    // send disconnect packet
    ret = mqtt_finalize_connection(context);

disconnect:
    // disconnect from mqtt broker
    mqtt_disconnect(context);

    return ret;
}
