#ifndef WORKERS_H
#define WORKERS_H

#include <stdint.h>

typedef struct {
    Scheduler_ctx *self;
    uint8_t queue_ID;
    rnd_bench_ctx_t* bench;
} worker_arg_t;

typedef struct {
    Scheduler_ctx *self;
    rnd_bench_ctx_t* bench;
} reaper_arg_t;

void* worker(void* arg);
void* reap_worker(void* arg);


#endif /* WORKERS_H */