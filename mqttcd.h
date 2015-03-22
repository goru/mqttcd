/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=4 : */

#include <stdio.h>
#include <stdlib.h>

#include "mqttcd_type.h"
#include "mqttcd_logger.h"
#include "mqttcd_arg.h"
#include "mqttcd_process.h"
#include "mqttcd_signal.h"
#include "mqtt_wrapper.h"

int main(int argc, char** argv);
int mqttcd(mqttcd_context_t* context);
int execute_message_handler(mqttcd_context_t* context, char* payload);
