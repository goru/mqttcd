/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=4 : */

#include "mqttcd_arg.h"

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
    int i;
    for (i = 0; i < sizeof(raw_options) / sizeof(raw_options[0]); i++) {
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

    int i;
    for (i = 0; i < sizeof(raw_options) / sizeof(raw_options[0]); i++) {
        if (*(raw_options[i]) != NULL) {
            free(*(raw_options[i]));
        }
    }

    return MQTTCD_SUCCEEDED;
}

