/* vim: set et fenc=utf-8 ff=unix ts=4 sw=4 sts=4 : */

#ifndef mqttcd_fork_h
#define mqttcd_fork_h

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#include "mqttcd_type.h"

int fork_current_process(int* pid);

#endif

