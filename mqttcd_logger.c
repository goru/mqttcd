/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=0 : */

#include "mqttcd_logger.h"

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
#ifdef DEBUG
    va_list arg;
    va_start(arg, format);
    vfprintf(context->logger, format, arg);
    va_end(arg);
    fflush(context->logger);
#endif
}

