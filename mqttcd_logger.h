/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=4 : */

#ifndef mqttcd_logger_h
#define mqttcd_logger_h

#include <stdio.h>
#include <stdarg.h>

#include "mqttcd_type.h"

int logger_open(mqttcd_context_t* context);
int logger_close(mqttcd_context_t* context);

void logger_error(mqttcd_context_t* context, const char* format, ...);
void logger_info(mqttcd_context_t* context, const char* format, ...);
void logger_debug(mqttcd_context_t* context, const char* format, ...);

#endif

