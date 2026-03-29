#ifndef WORKERS_H
#define WORKERS_H

#include <stdint.h>

typedef struct {
    Scheduler_ctx *self;
    uint8_t queue_ID;
} worker_arg_t;

void * worker(void* arg);

#endif /* WORKERS_H */