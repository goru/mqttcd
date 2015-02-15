/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=4 : */

#ifndef mqttcd_process_h
#define mqttcd_process_h

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>

#include "mqttcd_type.h"

int mqttcd_process_fork(int* pid);
int mqttcd_process_set_close_on_exec(int fd);

#endif

