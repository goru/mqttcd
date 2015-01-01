/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=4 : */

#include "mqttcd_logger.h"

int logger_open(mqttcd_context_t* context) {
    if (context->option.daemonize == 0) {
        context->logger = stdout;
    } else {
#ifndef ENABLE_SYSLOG
        FILE* fp = fopen("/var/tmp/mqttcd.log", "a");
        if (fp == NULL) {
            return MQTTCD_OPEN_FAILED;
        }
        context->logger = fp;
#else
        openlog(NULL, LOG_PID, LOG_DAEMON);
#endif
    }

    return MQTTCD_SUCCEEDED;
}

void logger_close(mqttcd_context_t* context) {
    if (context->option.daemonize == 0) {
        // do nothing
    } else {
#ifndef ENABLE_SYSLOG
        fclose(context->logger);
#else
        closelog();
#endif
    }
}

void logger_error(mqttcd_context_t* context, const char* format, ...) {
    va_list arg;
    va_start(arg, format);

#ifndef ENABLE_SYSLOG
    vfprintf(context->logger, format, arg);
    fflush(context->logger);
#else
    if (context->option.daemonize == 0) {
        vfprintf(context->logger, format, arg);
        fflush(context->logger);
    } else {
        vsyslog(LOG_ERR, format, arg);
    }
#endif

    va_end(arg);
}

void logger_notice(mqttcd_context_t* context, const char* format, ...) {
    va_list arg;
    va_start(arg, format);

#ifndef ENABLE_SYSLOG
    vfprintf(context->logger, format, arg);
    fflush(context->logger);
#else
    if (context->option.daemonize == 0) {
        vfprintf(context->logger, format, arg);
        fflush(context->logger);
    } else {
        vsyslog(LOG_NOTICE, format, arg);
    }
#endif

    va_end(arg);
}

void logger_debug(mqttcd_context_t* context, const char* format, ...) {
#ifdef ENABLE_DEBUG
    va_list arg;
    va_start(arg, format);

#ifndef ENABLE_SYSLOG
    vfprintf(context->logger, format, arg);
    fflush(context->logger);
#else
    if (context->option.daemonize == 0) {
        vfprintf(context->logger, format, arg);
        fflush(context->logger);
    } else {
        vsyslog(LOG_DEBUG, format, arg);
    }
#endif

    va_end(arg);
#endif
}

